#ifndef _BACKLIGHT_H
#define _BACKLIGHT_H

#include "sys/ctimer.h"

extern void backlight_on(uint8_t level, clock_time_t length);
extern void backlight_init();
extern void motor_on(uint8_t level, clock_time_t length);
#endif