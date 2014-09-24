#include "stlv.h"

#include <stdio.h>
#include <string.h>

#include "stlv_server.h"
#include "stlv_transport.h"
#include "btstack-config.h"
#include "debug.h"

static uint8_t s_btstack_type = BTSTACK_TYPE_UNKNOWN;
void set_btstack_type(uint8_t type)
{
    s_btstack_type = type;
}

uint8_t get_btstack_type()
{
    return s_btstack_type;
}

#define GET_PACKET_END(pack) (pack + STLV_HEAD_SIZE + get_body_length(pack))

int get_version(stlv_packet pack)
{
    return pack[HEADFIELD_VERSION];
}

int get_body_length(stlv_packet pack)
{
    return pack[HEADFIELD_BODY_LENGTH];
}

int get_sequence(stlv_packet pack)
{
    return pack[HEADFIELD_SEQUENCE];
}

int get_flag(stlv_packet pack)
{
    return pack[HEADFIELD_FLAG];
}

element_handle get_first_element(stlv_packet pack)
{
    if (get_body_length(pack) >= MIN_ELEMENT_SIZE)
        return pack + STLV_HEAD_SIZE;
    else
        return STLV_INVALID_HANDLE;
}

element_handle get_next_element(stlv_packet pack, element_handle handle)
{
    char type_buf[MAX_ELEMENT_TYPE_BUFSIZE] = {0};
    int  type_size = get_element_type(pack, handle, type_buf, sizeof(type_buf));
    int len = get_element_data_size(pack, handle, type_buf, type_size);
    if (GET_PACKET_END(pack) - (handle + len + type_size) >= MIN_ELEMENT_SIZE)
        return handle + len;
    else
        return STLV_INVALID_HANDLE;
}

element_handle get_first_sub_element(stlv_packet pack, element_handle parent)
{
    return get_element_data_buffer(pack, parent, 0, 0);
}

element_handle get_next_sub_element(stlv_packet pack, element_handle parent, element_handle handle)
{
    int parent_head_len = get_element_type(pack, parent, 0, 0) + 1;
    int parent_body_len = get_element_data_size(pack, parent, NULL, 0);

    int element_head_len = get_element_type(pack, handle, 0, 0) + 1;
    int element_body_len = get_element_data_size(pack, handle, NULL, 0);

    element_handle parent_end  = parent + parent_head_len  + parent_body_len;
    element_handle element_end = handle + element_head_len + element_body_len;

    if (parent_end - element_end < MIN_ELEMENT_SIZE)
        return STLV_INVALID_HANDLE;
    else
        return element_end;

}

int get_element_type(stlv_packet pack, element_handle handle, char* buf, int buf_size)
{
    UNUSED_VAR(pack);
    int cursor = 0;
    unsigned char* ptr = handle;
    while ((*ptr & 0x80) != 0)
    {
        if (cursor < buf_size && buf != NULL)
            buf[cursor] = (*ptr & ~0x80);
        ptr++;
        cursor++;
    }
    if (cursor < buf_size && buf != NULL)
        buf[cursor] = (*ptr & ~0x80);

    return cursor + 1;
}

int get_element_data_size(stlv_packet pack, element_handle handle, char* buf, int buf_size)
{
    if (buf != NULL && buf_size != 0)
        return handle[buf_size];
    else
        return handle[get_element_type(pack, handle, buf, buf_size)];
}

unsigned char* get_element_data_buffer(stlv_packet pack, element_handle handle, char* buf, int buf_size)
{
    if (buf != NULL && buf_size != 0)
        return handle + buf_size + 1;
    else
        return handle + get_element_type(pack, handle, buf, buf_size) + 1;
}

int filter_elements(
    stlv_packet pack,
    element_handle parent,
    element_handle begin,
    char* filter,
    int filter_size,
    element_handle* result
    )
{
    char type_buf[MAX_ELEMENT_TYPE_BUFSIZE];
    element_handle handle = begin;
    int mask  = 0;
    while (IS_VALID_STLV_HANDLE(handle))
    {
        get_element_type(pack, handle, type_buf, sizeof(type_buf));
        for (int i = 0; i < filter_size; ++i)
        {
            if (filter[i] == type_buf[0])
            {
                result[i] = handle;
                mask |= (1 << i);
                break;
            }
        }

        if (IS_VALID_STLV_HANDLE(parent))
            handle = get_next_sub_element(pack, parent, handle);
        else
            handle = get_next_element(pack, handle);
    }
    return mask;
}

#define PACKET_STATUS_NULL      0
#define PACKET_STATUS_SENDING   1
#define PACKET_STATUS_READY     2
#define PACKET_STATUS_PREPARING 3

typedef struct _stlv_packet_builder
{
    unsigned char trivial_head;
    unsigned char packet_data[STLV_PACKET_MAX_SIZE];

    element_handle stack[MAX_ELEMENT_NESTED_LAYER];
    int            stack_ptr;
    int            slot_id;
    void (*callback)(int);
    int  para;
    uint8_t status;
}stlv_packet_builder;

static stlv_packet_builder _packet;
static uint8_t _packet_used = 0;

#define GET_PACKET_BUILDER(p)     ((stlv_packet_builder*)p)
#define CAN_ADD_NEW_LAYER(bulder) (builder->stack_ptr < MAX_ELEMENT_NESTED_LAYER)

static int find_element(element_handle* vec, int size, element_handle val)
{
    for (int i = 0; i < size; i++)
    {
        if (vec[i] == val)
            return i;
    }
    return size;
}

static int inc_element_length(stlv_packet p, element_handle h, int size)
{
    int type_size = get_element_type(p, h, NULL, 0);
    h[type_size] += size;
    return type_size + 1;
}

static void inc_parent_elements_length(stlv_packet p, int size)
{
    stlv_packet_builder* builder = GET_PACKET_BUILDER(p);
    for (int i = 0; i < builder->stack_ptr; i++)
        inc_element_length(p, builder->stack[i], size);
}

stlv_packet create_packet()
{
    if (_packet_used)
        return NULL;

    _packet_used = 1;

    stlv_packet_builder* builder = &_packet;
    builder->packet_data[HEADFIELD_VERSION]     = 1;
    builder->packet_data[HEADFIELD_FLAG]        = 0x80;
    builder->packet_data[HEADFIELD_BODY_LENGTH] = 0;
    builder->packet_data[HEADFIELD_SEQUENCE]    = 0;
    builder->stack_ptr = 0;
    builder->callback  = 0;
    builder->para      = 0;
    builder->status    = PACKET_STATUS_PREPARING;

    return _packet.packet_data;
}

static void sent_complete(int para)
{
    //handle callback
    log_info("send_complete()\n");
    _packet_used = 0;
    stlv_packet_builder* cur_builder = &_packet;
    if (cur_builder->callback != 0)
        cur_builder->callback(cur_builder->para);

    //try send next packet
    //log_info("send_complete para=%d, %d-%d/%d\n", para, _packet_reader, _packet_writer, BUILDER_COUNT);
    //if (_packet_reader == _packet_writer)
    //    return;

    //stlv_packet_builder* builder = &_packet[_packet_reader];
    //stlv_packet p = builder->packet_data;
    //spp_register_task(p, p[HEADFIELD_BODY_LENGTH] + STLV_HEAD_SIZE, sent_complete, builder->slot_id);
}

int send_packet(stlv_packet p, void (*callback)(int), int para)
{
    stlv_packet_builder* builder = (stlv_packet_builder*)(p - 1);
    builder->callback = callback;
    builder->para     = para;
    return spp_register_task(p, p[HEADFIELD_BODY_LENGTH] + STLV_HEAD_SIZE, sent_complete, builder->slot_id);
}

int  set_version(stlv_packet p, int version)
{
    p[HEADFIELD_VERSION] = (unsigned char)version;
    return 1;
}

int  set_body_length(stlv_packet p, int len)
{
    p[HEADFIELD_BODY_LENGTH] = (unsigned char)len;
    return 1;
}

int  set_sequence(stlv_packet p, int sn)
{
    p[HEADFIELD_SEQUENCE] = (unsigned char)sn;
    return 1;
}

int  set_flag(stlv_packet p, int flag)
{
    p[HEADFIELD_FLAG] = (unsigned char)flag;
    return 1;
}

static void set_element_type(stlv_packet p, element_handle h, char* type_buf, int buf_len)
{
    for (int i = 0; i < buf_len - 1; i++)
        h[i] = (unsigned char)(type_buf[i] | 0x80);
    h[buf_len - 1] = (unsigned char)(type_buf[buf_len - 1]);

    p[HEADFIELD_BODY_LENGTH] += (buf_len + 1);
    inc_parent_elements_length(p, buf_len + 1);
    h[buf_len] = 0; //set element length = 0
}

element_handle append_element(stlv_packet p, element_handle parent, char* type_buf, int buf_len)
{
    stlv_packet_builder* builder = GET_PACKET_BUILDER(p);
    if (IS_VALID_STLV_HANDLE(parent))
    {
        int pos = find_element(builder->stack, builder->stack_ptr, parent);
        if (pos != builder->stack_ptr)
        {
            builder->stack_ptr = pos + 1;
        }
        else
        {
            if (!CAN_ADD_NEW_LAYER(builder))
                return STLV_INVALID_HANDLE;
            else
                builder->stack[builder->stack_ptr++] = parent;
        }
    }
    else
    {
        builder->stack_ptr = 0;
    }

    int len = get_body_length(p) + STLV_HEAD_SIZE;
    if (len >= STLV_PACKET_MAX_BODY_SIZE)
        return STLV_INVALID_HANDLE;

    element_handle h = p + len;

    set_element_type(p, h, type_buf, buf_len);

    return h;
}

int element_append_char(stlv_packet p, element_handle h, char data)
{
    return ELEMENT_APPEND_TYPE(p, h, data);
}

int element_append_short(stlv_packet p, element_handle h, short data)
{
    return ELEMENT_APPEND_TYPE(p, h, data);
}

int element_append_int(stlv_packet p, element_handle h, int data)
{
    return ELEMENT_APPEND_TYPE(p, h, data);
}

int element_append_long(stlv_packet p, element_handle h, long data)
{
    return ELEMENT_APPEND_TYPE(p, h, data);
}

int element_append_data(stlv_packet p, element_handle h, uint8_t* data_buf, int buf_len)
{
    unsigned char* body_start = get_element_data_buffer(p, h, NULL, 0);
    for (int i = 0; i < buf_len; i++)
        body_start[i] = data_buf[i];

    inc_element_length(p, h, buf_len);
    p[HEADFIELD_BODY_LENGTH] += buf_len;
    inc_parent_elements_length(p, buf_len);
    return buf_len;
}

int element_append_string(stlv_packet p, element_handle h, char* data)
{
    int len = strlen(data);
    return element_append_data(p, h, (unsigned char*)data, len);
}

