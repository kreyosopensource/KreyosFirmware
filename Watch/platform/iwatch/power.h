#ifndef _POWER_H
#define _POWER_H

#define MODULE_LCD BIT0
#define MODULE_BT  BIT1
#define MODULE_CODEC BIT2

void power_pin(uint8_t module);
void power_unpin(uint8_t module);

#endif