#ifndef _HFP_H_
#define _HFP_H_
#include "btstack/utils.h"

#define HFP_T_CIEV 1
#define HFP_T_RING 2

#define HFP_CIND_UNKNOWN	-1
#define HFP_CIND_NONE		0
#define HFP_CIND_SERVICE	1
#define HFP_CIND_CALL		2
#define HFP_CIND_CALLSETUP	3
#define HFP_CIND_CALLHELD	4
#define HFP_CIND_SIGNAL		5
#define HFP_CIND_ROAM		6
#define HFP_CIND_BATTCHG	7

/* call indicator values */
#define HFP_CIND_CALL_NONE	0
#define HFP_CIND_CALL_ACTIVE	1

/* callsetup indicator values */
#define HFP_CIND_CALLSETUP_NONE		0
#define HFP_CIND_CALLSETUP_INCOMING	1
#define HFP_CIND_CALLSETUP_OUTGOING	2
#define HFP_CIND_CALLSETUP_ALERTING	3

/* service indicator values */
#define HFP_CIND_SERVICE_NONE		0
#define HFP_CIND_SERVICE_AVAILABLE	1

extern void hfp_init();
extern void hfp_open(const bd_addr_t *remote_addr, uint8_t port);

extern uint8_t hfp_enable_voicerecog(uint8_t onoff);
extern uint8_t hfp_accept_call(uint8_t);
extern void hfp_callback(uint8_t type, uint8_t index, uint8_t value);
extern uint8_t hfp_getstatus(uint8_t ind);
extern uint8_t hfp_connected();
extern bd_addr_t* hfp_remote_addr();
#endif