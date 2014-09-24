#ifndef _TEST_H_
#define _TEST_H_

#include "contiki.h"

uint8_t test_button(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_motor(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_light(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_lcd(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_reboot(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_ant(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_mpu6050(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_bluetooth(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_dut(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_ble(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_codec(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_googlenow(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_cleardata(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_builddata(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_sleep(uint8_t ev, uint16_t lparam, void* rparam);
uint8_t test_sportsdata(uint8_t ev, uint16_t lparam, void* rparam);
#endif
