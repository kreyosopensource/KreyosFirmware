
#include "stlv_test_stub.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static send_pack_stub_t * _send_pack_stub = 0;
static int _send_pack_stub_count = 0;

int get_send_pack_stub_count()
{
    return _send_pack_stub_count;
}

send_pack_stub_t* get_send_pack_stub()
{
    return _send_pack_stub;
}

send_pack_stub_t* get_next_send_pack_stub(send_pack_stub_t* node)
{
    return node->next;
}

void dump_stub()
{
    send_pack_stub_t* head = get_send_pack_stub();
    for (int i = 0; i < _send_pack_stub_count; ++i)
    {
        printf("(%p<-%p->%p) %d\n", head->prev, head, head->next, i);
        head = get_next_send_pack_stub(head);
    }
}

static send_pack_stub_t* append_stub(send_pack_stub_t** phead)
{
    _send_pack_stub_count++;

    send_pack_stub_t* new_node =
        (send_pack_stub_t*)malloc(sizeof(send_pack_stub_t));
    new_node->next = new_node->prev = new_node;

    send_pack_stub_t* head = *phead;
    if (head != 0)
    {
        head->prev->next = new_node;
        new_node->prev   = head->prev;
        head->prev       = new_node;
        new_node->next   = head;
    }
    else
    {
        *phead = new_node;
    }
    return new_node;
}

static void remove_stub(send_pack_stub_t* node)
{

    _send_pack_stub_count--;
    if (node->next == node && node->prev == node)
        return;

    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = node;
    node->prev = node;
}

send_pack_stub_t* init_send_pack_stub()
{
    send_pack_stub_t* head = _send_pack_stub;
    if (head == 0)
        return 0;
    while (_send_pack_stub_count)
    {
        send_pack_stub_t* node = get_next_send_pack_stub(head);
        remove_stub(head);
        free(head);
        head = node;
    }
    _send_pack_stub = 0;
    return _send_pack_stub;
}

int test_send_internal(uint16_t rfcomm_cid, uint8_t *data, uint16_t len)
{
    printf("test_send_internal:len=%d\n", len);
    send_pack_stub_t* new_node = append_stub(&_send_pack_stub);

    new_node->channelId = rfcomm_cid;
    new_node->len       = len;
    memcpy(new_node->data, data, len);

    return 0;
}

void hex_dump(uint8_t* data, uint16_t len)
{
    int inline_cnt = 0;
    for (int i = 0; i < len; ++i)
    {
        inline_cnt++;
        printf("0x%02x, ", data[i]);
        if (inline_cnt == 9)
        {
            printf("\n");
            inline_cnt = 0;
        }
    }
    printf("\n");
}

