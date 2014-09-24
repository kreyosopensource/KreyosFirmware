#ifndef _AVCTP_H_
#define _AVCTP_H_

#include "btstack/utils.h"

#define PSM_AVCTP 23

#define AVC_MTU 512
#define AVC_HEADER_LENGTH 3

/* ctype entries */
#define AVC_CTYPE_CONTROL		0x0
#define AVC_CTYPE_STATUS		0x1
#define AVC_CTYPE_NOTIFY		0x3
#define AVC_CTYPE_NOT_IMPLEMENTED	0x8
#define AVC_CTYPE_ACCEPTED		0x9
#define AVC_CTYPE_REJECTED		0xA
#define AVC_CTYPE_STABLE		0xC
#define AVC_CTYPE_CHANGED		0xD
#define AVC_CTYPE_INTERIM		0xF

/* opcodes */
#define AVC_OP_VENDORDEP		0x00
#define AVC_OP_UNITINFO			0x30
#define AVC_OP_SUBUNITINFO		0x31
#define AVC_OP_PASSTHROUGH		0x7c

/* subunits of interest */
#define AVC_SUBUNIT_PANEL		0x09

/* operands in passthrough commands */
#define VOL_UP_OP			0x41
#define VOL_DOWN_OP			0x42
#define MUTE_OP				0x43
#define PLAY_OP				0x44
#define STAVC_OP_OP			0x45
#define PAUSE_OP			0x46
#define RECORD_OP			0x47
#define REWIND_OP			0x48
#define FAST_FORWARD_OP			0x49
#define EJECT_OP			0x4a
#define FORWARD_OP			0x4b
#define BACKWARD_OP			0x4c

extern void avctp_init();
extern void avctp_connect(bd_addr_t remote_addr);
extern void avctp_disconnect();
extern int avctp_connected();
extern void avctp_register_pid(uint16_t pid, void (*handler)(uint8_t code, uint8_t *packet, uint16_t size));

extern int avctp_send_vendordep(uint8_t code, uint8_t subunit,
				uint8_t *operands, uint8_t operand_count);
extern int avctp_send_passthrough(uint8_t op);

#endif