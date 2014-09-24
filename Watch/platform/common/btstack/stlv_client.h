
#ifndef _STLV_CLIENT_H_
#define _STLV_CLIENT_H_

#include <stdint.h>

//-----------------------message sender------------------------
void send_echo(uint8_t* data, uint8_t size);

//return 0 if success
int begin_send_file(char* name);
int send_file_data(int fd, uint8_t* data, uint8_t size, void (*callback)(int), int para);
void end_send_file(int fd);

//the normal interface to send file
void send_file(char* name);

//files should be ';' seperated file names
void send_file_list(char* files);

//sports data
#define SPORTS_DATA_FLAG_START 0x01
#define SPORTS_DATA_FLAG_STOP  0x02
#define SPORTS_DATA_FLAG_PRE   0x04
#define SPORTS_DATA_FLAG_SYNC  0x08
#define SPORTS_DATA_FLAG_BIKE  0x10
#define SPORTS_DATA_FLAG_RUN   0x20
void send_sports_data(uint8_t id, uint8_t flag, uint8_t* meta, uint32_t* data, uint8_t size);
void send_sports_grid(uint8_t* data, uint8_t size);

//notification
void send_notification(uint8_t sub_type, char* from, char* message);

//alarm
#define ALARM_MODE_NO_EXIST 0x00
#define ALARM_MODE_DISABLE  0x01
#define ALARM_MODE_ONCE     0x02
#define ALARM_MODE_HOURLY   0x03
#define ALARM_MODE_DAILY    0x04
#define ALARM_MODE_WEEKLY   0x05
#define ALARM_MODE_MONTHLY  0x06
#define ALARM_VIBRATE_FLAG  0x10

typedef struct _alarm_conf_t
{
    uint8_t id;
    uint8_t mode;
    uint8_t day_of_month;
    uint8_t day_of_week;
    uint8_t hour;
    uint8_t minute;
}alarm_conf_t;

void send_alarm_conf(alarm_conf_t* data);

//device id
void send_device_id(char* device_id);
void launch_google_now();
void send_firmware_version(char* version);
void send_daily_activity(uint16_t time, uint16_t steps, uint32_t calories, uint32_t distance);

#endif

