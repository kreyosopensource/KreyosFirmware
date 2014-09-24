
#ifndef _STLV_TEST_STUB_H_
#define _STLV_TEST_STUB_H_

#include <stdint.h>

typedef struct _send_pack_stub_t
{
    uint16_t channelId;
    uint8_t  data[4096];
    uint16_t len;
    struct _send_pack_stub_t* next;
    struct _send_pack_stub_t* prev;
}send_pack_stub_t;

int test_send_internal(uint16_t rfcomm_cid, uint8_t *data, uint16_t len);

send_pack_stub_t* get_send_pack_stub();
send_pack_stub_t* get_next_send_pack_stub();
send_pack_stub_t* init_send_pack_stub();
int get_send_pack_stub_count();

void hex_dump(uint8_t* data, uint16_t len);

#endif

