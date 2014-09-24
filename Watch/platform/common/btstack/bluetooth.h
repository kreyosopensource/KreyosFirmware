#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_
#include "btstack/utils.h"

#define BT_INITIALIZED  1
#define BT_CONNECTED    2
#define BT_DISCONNECTED 3
#define BT_PAIRED       4
#define BT_SHUTDOWN     5

extern void bluetooth_init();
extern void bluetooth_start();
extern void bluetooth_shutdown();
extern void bluetooth_discoverable(uint8_t onoff);
extern uint8_t bluetooth_running();
extern const char* bluetooth_address();

extern void codec_setvolume(uint8_t level);
extern uint8_t codec_changevolume(int8_t diff);
extern uint8_t codec_getvolume();
extern void codec_wakeup();
extern void codec_init();
extern void codec_shutdown();
extern void codec_suspend();

PROCESS_NAME(bluetooth_process);

extern void bluetooth_platform_init(void);
extern void bluetooth_platform_shutdown(void);
#endif
