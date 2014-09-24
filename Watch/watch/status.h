
#ifndef _WINDOW_STATUS_H_
#define _WINDOW_STATUS_H_

#include <stdint.h>

#define WS_NORMAL 0x0000
#define WS_SPORTS 0x000l
#define WS_NOTIFY 0x0002

uint16_t add_watch_status(uint16_t value);
uint16_t del_watch_status(uint16_t value);
uint16_t set_watch_status(uint16_t value);
uint16_t get_watch_status();

void record_last_action();
uint16_t check_idle_time();

#endif
