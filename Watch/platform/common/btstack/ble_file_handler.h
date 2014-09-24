#ifndef _BLE_HANDLER_FILE_H_
#define _BLE_HANDLER_FILE_H_

#include "stdint.h"

#define FD_GET_COMMAND(buf)   (buf[0])
#define FD_GET_BLOCKID(buf)   (buf[1])
#define FD_GET_FILENAME(buf)  ((char*)&buf[4])

#define FD_SET_COMMAND(buf, cmd)        buf[0] = cmd
#define FD_SET_BLOCKID(buf, blockid)    buf[1] = blockid
#define FD_SET_FILENAME(buf, filename)  strcpy((char*)&buf[4], filename)

uint16_t FD_GET_BLOCKSIZE(uint8_t* buf);
void FD_SET_BLOCKSIZE(uint8_t* buf, uint16_t blocksize);

uint8_t get_file_mode();

void ble_write_file_desc(uint8_t* buffer, uint16_t buffer_size);
void ble_read_file_desc(uint8_t * buffer, uint16_t buffer_size);
void ble_read_file_data(uint8_t* buffer, uint8_t buffer_size);
void ble_write_file_data(uint8_t* buffer, uint8_t buffer_size);

#endif
