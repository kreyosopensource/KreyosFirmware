#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "rtc.h"

void system_init();

void system_rebootToNormal();
uint8_t system_testing();
uint8_t system_retail();
void system_ready();
void system_shutdown(int shipping);
uint8_t system_locked();
void system_resetfactory();
void system_unlock();
void system_reset();
void system_restore();
const uint8_t *system_getserial();
void system_setemerging();

extern struct pesistent_data
{
	uint16_t checksum;
	uint16_t step_count;
	uint32_t step_time;
	uint32_t step_cal;
	uint32_t step_dist;

	struct datetime now;
}globaldata;

#endif