#pragma once

#define BAUD_4800 0x01
// 9600
#define BAUD_9600 0x02
// 19200
#define BAUD_19200 0x03
// 38400
#define BAUD_38400 0x04
// 57600
#define BAUD_57600 0x05
// 115200
#define BAUD_115200 0x06


void uart_changerate(char rate);
void uart_init(char rate);
uint8_t uart_getByte();
void uart_sendByte(uint8_t byte);
