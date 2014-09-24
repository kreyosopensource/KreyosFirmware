
#include <stdio.h>
#include <string.h>

#include "stlv.h"
#include "contiki.h"
#include "window.h"
#include "rtc.h"
#include "cfs/cfs.h"
#include "stlv_client.h"
#include "stlv_handler.h"
#include "btstack/include/btstack/utils.h"
#include "watch/sportsdata.h"
#include "btstack-config.h"
#include "system.h"
#include "debug.h"
#include "pedometer/pedometer.h"

extern void spp_sniff(int onoff);

void handle_echo(uint8_t* data, int data_len)
{
    send_echo(data, data_len);
}

void handle_clock(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    rtc_setdate(2000 + year, month + 1, day);
    rtc_settime(hour, minute, second);

    window_invalid(NULL);
    status_invalid();
}

static uint8_t _phone_info[2] = {0};
void handle_phone_info(uint8_t phone_type, uint8_t phone_ver)
{
    log_info("phone info: type=%x, ver=%x\n", phone_type, phone_ver);
    _phone_info[0] = phone_type;
    _phone_info[1] = phone_ver;

    send_firmware_version(FWVERSION);
    spp_sniff(1); //turn on sniff for power save
}

uint8_t get_phone_type()
{
    return _phone_info[0];
}

#define ICON_FACEBOOK 's'
#define ICON_TWITTER  't'
#define ICON_MSG      'u'

void handle_message(uint8_t msg_type, char* ident, char* message)
{
    uint8_t icon;
    switch (msg_type)
    {
    case ELEMENT_TYPE_MESSAGE_SMS:
        icon = ICON_MSG;
        break;
    case ELEMENT_TYPE_MESSAGE_FB:
        icon = ICON_FACEBOOK;
        break;
    case ELEMENT_TYPE_MESSAGE_TW:
        icon = ICON_TWITTER;
        break;

    case ELEMENT_TYPE_MESSAGE_WEATHER:
    case ELEMENT_TYPE_MESSAGE_BATTERY:
    case ELEMENT_TYPE_MESSAGE_CALL:
    case ELEMENT_TYPE_MESSAGE_REMINDER:
        //TODO: impelment all these
        icon = ICON_MSG;
        break;
    case ELEMENT_TYPE_MESSAGE_RANGE:
        //TODO: this is special: phone out of range is merely an option
        //in message, if the value is "off" turn off the alarm
        //in message, if the value is "on" turn on the alarm
        break;
    default:
        return;
        break;
    }

    window_notify(ident, message, NOTIFY_OK, icon);

}

static uint32_t offset;
#define FIRMWARE_HD 784
extern void WriteFirmware(void* data, uint32_t offset, int size);
extern void EraseFirmware();
extern int CheckUpgrade(void);
extern void system_reset();

int handle_file_begin(char* name)
{
    int fd;
    log_info("handle_file_begin(%s)\n", name);

    if (strcmp(name, "firmware") == 0)
    {
        EraseFirmware();
        fd = FIRMWARE_HD;
        offset = 0;
        process_post(ui_process, EVENT_FIRMWARE_UPGRADE, (void*)offset);
    }
    else
    {
        fd = cfs_open(name, CFS_WRITE);
        if (fd == -1)
        {
            log_info("Open file %s failed\n", name);
        }
    }

    return fd;
}

int handle_file_data(int fd, uint8_t* data, uint8_t size)
{
    log_info("handle_file_data(%x, %d)\n", fd, size);
    if (fd == FIRMWARE_HD)
    {
        WriteFirmware(data, offset, size);
        offset += size;
        process_post(ui_process, EVENT_FIRMWARE_UPGRADE, (void*)offset);
    }
    else if (fd != -1)
    {
        return cfs_write(fd, data, size);
    }
    
    return 0;
}

void handle_file_end(int fd)
{
    log_info("handle_file_end(%x)\n", fd);
    if (fd == FIRMWARE_HD)
    {
        process_post(ui_process, EVENT_FIRMWARE_UPGRADE, (void*)-1);
    }
    else if (fd != -1)
        cfs_close(fd);
}

void handle_get_file(char* name)
{
    log_info("handle_get_file(%s)\n", name);
    transfer_file(name);
}

void handle_get_activity()
{
    uint32_t size = 0;
    char* fn = get_data_file(&size);
    if (fn == NULL)
    {
        log_info("handle_get_activity() no valid data\n");
        return;
    }
    log_info("handle_get_activity() return %s\n", fn);

    transfer_file(fn);
}

static char* filter_filename_by_prefix(char* prefix, char* filename)
{
    char* pp = prefix;
    char* pf = filename;
    while (*pp != '\0' && *pf != '\0')
    {
        if (*pp != *pf)
        {
            return 0;
        }
        ++pp;
        ++pf;
    }
    if (*pf != '\0')
        return pf;
    else
        return 0;
}

//TODO: help verify
void handle_list_file(char* prefix)
{
    log_info("handle_list_file(%s)\n", prefix);

    char buf[200] = "";
    uint8_t buf_size = 0;
    struct cfs_dir dir;
    int ret = cfs_opendir(&dir, "");
    if (ret == -1)
    {
        log_info("cfs_opendir() failed: %d\n", ret);
        return;
    }

    while (ret != -1)
    {
        struct cfs_dirent dirent;
        ret = cfs_readdir(&dir, &dirent);
        if (ret != -1)
        {
            log_info("file:%s, %d\n", dirent.name, dirent.size);

            char* short_name = filter_filename_by_prefix(prefix, dirent.name);
            uint8_t len = strlen(short_name) + 1;
            if (buf_size + len >= sizeof(buf) - 1)
                break;
            strcat(buf, short_name);
            strcat(buf, ";");
            buf_size += len;
        }
    }

    cfs_closedir(&dir);

    send_file_list(buf);

}

void handle_remove_file(char* name)
{
    log_info("handle_remove_file(%s)\n", name);
    cfs_remove(name);
}

void handle_get_device_id()
{
    //TODO: return device id
    log_info("handle_get_device_id()\n");
}

void handle_gps_info(uint16_t spd, uint16_t alt, uint32_t distance)
{
    log_info("handle_gps_info(%d, %d, %d)\n", spd, alt, distance);
    uint32_t lspd = (spd == 0xffff ? 0xffffffff : spd);
    uint32_t lalt = (alt == 0xffff ? 0xffffffff : alt);

    window_postmessage(EVENT_SPORT_DATA, SPORTS_SPEED,    (void*)lspd);
    window_postmessage(EVENT_SPORT_DATA, SPORTS_ALT,      (void*)lalt);
    window_postmessage(EVENT_SPORT_DATA, SPORTS_DISTANCE, (void*)distance);
}

#define MAX_FILE_NAME_SIZE 32 + 1
typedef struct _file_reader_t
{
    int      read_fd;
    int      send_fd;
    uint16_t read_cursor;
    uint16_t file_size;
    char     file_name[MAX_FILE_NAME_SIZE];
    uint8_t  sent_bytes;
}file_reader_t;

static file_reader_t _f_reader;
static void init_file_reader(file_reader_t* reader)
{
    reader->send_fd      = 0;
    reader->read_fd      = 0;
    reader->read_cursor  = 0;
    reader->file_size    = 0;
    reader->file_name[0] = '\0';
    reader->sent_bytes   = 0;
}

static void send_file_block(int para);
static void send_file_callback(int para);

static void send_file_block(int para)
{
    uint8_t buf[200];
    cfs_seek(_f_reader.read_fd, _f_reader.read_cursor, CFS_SEEK_SET);
    _f_reader.sent_bytes = cfs_read(_f_reader.read_fd, buf, sizeof(buf));
    send_file_data(para, buf, _f_reader.sent_bytes, send_file_callback, para);
    log_info("send_file_block(%d), %d, %d\n", para, _f_reader.sent_bytes, _f_reader.read_cursor);

}

static void send_file_callback(int para)
{
    log_info("send_file_callback(%d), %d/%d\n", para, _f_reader.read_cursor, _f_reader.file_size);
    _f_reader.read_cursor += _f_reader.sent_bytes;
    _f_reader.sent_bytes = 0;
    if (_f_reader.read_cursor >= _f_reader.file_size)
    {
        end_send_file(_f_reader.send_fd);
        cfs_close(_f_reader.read_fd);

        if (!is_active_data_file(_f_reader.file_name))
        {
            remove_data_file(_f_reader.file_name);
        }
        return;
    }

    send_file_block(para);
}

int transfer_file(char* filename)
{
    int fd_read = cfs_open(filename, CFS_READ);
    if (fd_read == -1)
    {
        log_error("cfs_open(%s) failed\n", filename); 
        return -1;
    }

    cfs_offset_t pos = -1;
    if ((pos = cfs_seek(fd_read, 0, CFS_SEEK_END)) == -1)
    {
        log_error("cfs_seek(%s) to file end failed", filename); 
        cfs_close(fd_read);
        return -1;
    }

    if ((pos = cfs_seek(fd_read, 0, CFS_SEEK_SET)) == -1)
    {
        log_error("cfs_seek(%s) to file begin failed", filename); 
        cfs_close(fd_read);
        return -1;
    }

    init_file_reader(&_f_reader);
    strcpy(_f_reader.file_name, filename);
    _f_reader.send_fd = begin_send_file(filename);
    _f_reader.read_fd = fd_read;
    _f_reader.file_size = (uint16_t)pos;

    send_file_block(_f_reader.send_fd);

    return 0;
}

void handle_get_sports_data(uint16_t *data, uint8_t numofdata)
{
    stlv_packet p = create_packet();
    if (p == NULL)
        return;
    element_handle h = append_element(p, NULL, "A", 1);

    element_append_data(p, h, (uint8_t*)&data, sizeof(uint16_t) * numofdata);
    send_packet(p, 0, 0);
}

void handle_get_sports_grid()
{
    stlv_packet p = create_packet();
    if (p == NULL)
        return;
    element_handle h = append_element(p, NULL, "R", 1);
    ui_config* config = window_readconfig();
    element_append_data(p, h, (uint8_t*)&config->sports_grid_data, sizeof(config->sports_grid_data));
    log_info("send_sports_grid(%d)\n", sizeof(config->sports_grid_data));
    send_packet(p, 0, 0);
}

void handle_alarm(alarm_conf_t* para)
{
    log_info("set alarm:\n");
    log_info("  id           = %d\n", para->id);
    log_info("  mode         = %d\n", para->mode);
    log_info("  day_of_month = %d\n", para->day_of_month);
    log_info("  day_of_week  = %d\n", para->day_of_week);
    log_info("  hour         = %d\n", para->hour);
    log_info("  minute       = %d\n", para->minute);

    if (para->id >= MAX_ALARM_COUNT)
        return;

    switch(para->mode)
    {
        case ALARM_MODE_DISABLE:
            {
                ui_config* conf = window_readconfig();
                conf->alarms[para->id].flag    = 0;
                conf->alarms[para->id].hour    = 0;
                conf->alarms[para->id].minutes = 0;
                window_writeconfig();
                window_loadconfig();
            }
            break;
        case ALARM_MODE_NO_EXIST:
        case ALARM_MODE_MONTHLY:
        case ALARM_MODE_HOURLY:
        case ALARM_MODE_WEEKLY:
        case ALARM_MODE_ONCE:
            // no way
            break;
        case ALARM_MODE_DAILY:
            {
                ui_config* conf = window_readconfig();
                conf->alarms[para->id].flag    = 1;
                conf->alarms[para->id].hour    = para->hour;
                conf->alarms[para->id].minutes = para->minute;
                window_writeconfig();
                window_loadconfig();
            }
            break;
    }
}

void handle_gesture_control(uint8_t flag, uint8_t action_map[])
{
    ui_config* online_config = window_readconfig();
    if (online_config != NULL)
    {
        online_config->gesture_flag = flag;
        for (uint8_t i = 0; i < 4; ++i)
            online_config->gesture_map[i] = action_map[i];
        window_writeconfig();
        window_loadconfig();
    }
}

void handle_set_watch_config(ui_config* new_config)
{
    //TODO: help check this




    ui_config* config = window_readconfig();
    if (config != NULL)
    {
        memcpy(config, new_config, sizeof(ui_config));

        //adjust values: big endian to little endian
        uint8_t* p = (uint8_t*)new_config;
        config->signature     = READ_NET_32(p, 0);
        config->goal_steps    = READ_NET_16(p, 4);
        config->goal_distance = READ_NET_16(p, 6);
        config->goal_calories = READ_NET_16(p, 8);
        config->lap_length    = READ_NET_16(p, 10);

        if (config->weight < 20) config->weight = 20;
        if (config->height < 60) config->height = 60;

        log_info("set_watch_config:\n");
        log_info("  signature     = %x\n", config->signature);
        log_info("  default_clock = %d\n", config->default_clock); // 0 - analog, 1 - digit
        log_info("  analog_clock  = %d\n", config->analog_clock);  // num : which lock face
        log_info("  digit_clock   = %d\n", config->digit_clock);   // num : which clock face
        log_info("  sports_grid   = %d\n", config->sports_grid);   // 0 - 3 grid, 1 - 4 grid, 2 - 5 grid
        log_info("  sports_grids  = %d, %d, %d, %d, %d\n",
                config->sports_grid_data[0],
                config->sports_grid_data[1],
                config->sports_grid_data[2],
                config->sports_grid_data[3],
                config->sports_grid_data[4]);
        log_info("  is_ukuint     = %02x\n", config->is_ukuint);
        log_info("  goal_steps    = %d\n", config->goal_steps);
        log_info("  goal_distance = %d\n", config->goal_distance);
        log_info("  goal_calories = %d\n", config->goal_calories);
        log_info("  weight        = %d\n", config->weight); // in kg
        log_info("  height        = %d\n", config->height); // in cm
        log_info("  circumference = %d\n", config->circumference);
        log_info("  lap_len       = %d\n", config->lap_length);


        window_writeconfig();
        window_loadconfig();
    }
}

void handle_unlock_watch()
{
    system_unlock();
}

void handle_daily_activity()
{
    send_daily_activity(
        ped_get_time(),
        ped_get_steps(),
        ped_get_calorie(),
        ped_get_distance()
        );
}
