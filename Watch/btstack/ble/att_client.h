#ifndef _ATT_CLIENT_H_
#define _ATT_CLIENT_H_
#include <btstack/btstack.h>
#include <stdint.h>
typedef uint16_t att_connection_t ;

typedef struct le_peripheral_event{
    uint8_t   type;
    uint8_t status;
} le_peripheral_event_t;

typedef struct le_service{
    uint16_t start_group_handle;
    uint16_t end_group_handle;
    uint16_t uuid16; 
    uint8_t  uuid128[16]; 
} le_service_t;

typedef struct le_service_event{
    uint8_t  type;
    le_service_t service; 
} le_service_event_t;

uint16_t report_gatt_services(att_connection_t *conn, uint8_t * packet,  uint16_t size);
void     att_client_notify(uint16_t handle, uint8_t *data, uint16_t length);
void report_write_done(att_connection_t *conn, uint16_t handle);
uint16_t report_service_characters(att_connection_t *conn, uint8_t * packet,  uint16_t size);

void att_fetch_next(uint32_t uid, uint32_t combine);
#endif