
#ifndef _STLV_TRANSPORT_H_
#define _STLV_TRANSPORT_H_

#include <stdint.h>

#define SPP_SENDER_NULL     0
#define SPP_SENDER_READY    1
#define SPP_SENDER_SENDING  2
#define SPP_SENDER_SENT     3

#define SPP_PACKET_MTU  64
#define SPP_PACKET_SIZE SPP_PACKET_MTU - 1

#define SPP_FLAG_BEGIN       0x01
#define SPP_FLAG_END         0x02
#define SPP_FLAG_BEGIN_END   (SPP_FLAG_BEGIN | SPP_FLAG_END)


uint8_t tryToSend(void);
int spp_register_task(uint8_t* buf, int size, void (*callback)(int), int para);

short handle_stvl_transport(uint8_t* packet, uint16_t size);
void reset_stlv_transport_buffer();
short get_stlv_transport_buffer_size();
uint8_t* get_stlv_transport_buffer();

#endif

