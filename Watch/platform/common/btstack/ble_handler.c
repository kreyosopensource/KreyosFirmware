
#include "ble_handler.h"

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "window.h"
#include "btstack/include/btstack/utils.h"
#include "rtc.h"
#include "stlv_handler.h"
#include "btstack-config.h"
#include "debug.h"
#include "watch/sportsdata.h"
#include "cfs/cfs.h"
#include "ble_file_handler.h"
#include "rtc.h"
#include "system.h"
#include "pedometer/pedometer.h"

//
// list mapping between characteristics and handles
//

static const ble_handle_t s_ble_handle_table[] = {
    /*     characteristic, name                          type                       size*/
    DEF_BLE_HANDLE("fff1", BLE_HANDLE_UNLOCK_WATCH,      BLE_HANDLE_TYPE_INT8_ARR,  1),
    DEF_BLE_HANDLE("fff2", BLE_HANDLE_FW_VERSION,        BLE_HANDLE_TYPE_STRING,    20),
    DEF_BLE_HANDLE("fff3", BLE_HANDLE_DATETIME,          BLE_HANDLE_TYPE_INT8_ARR,  6),
    DEF_BLE_HANDLE("fff4", BLE_HANDLE_ALARM_0,           BLE_HANDLE_TYPE_INT8_ARR,  2),
    DEF_BLE_HANDLE("fff5", BLE_HANDLE_ALARM_1,           BLE_HANDLE_TYPE_INT8_ARR,  2),
    DEF_BLE_HANDLE("fff6", BLE_HANDLE_ALARM_2,           BLE_HANDLE_TYPE_INT8_ARR,  2),
    DEF_BLE_HANDLE("fff7", BLE_HANDLE_SPORTS_GRID,       BLE_HANDLE_TYPE_INT8_ARR,  4),
    DEF_BLE_HANDLE("fff8", BLE_HANDLE_SPORTS_DATA,       BLE_HANDLE_TYPE_INT32_ARR, 5),
    DEF_BLE_HANDLE("fff9", BLE_HANDLE_SPORTS_DESC,       BLE_HANDLE_TYPE_INT32_ARR, 2),
    DEF_BLE_HANDLE("ff10", BLE_HANDLE_DEVICE_ID,         BLE_HANDLE_TYPE_INT32_ARR, 1),
    DEF_BLE_HANDLE("ff11", BLE_HANDLE_FILE_DESC,         BLE_HANDLE_TYPE_STRING,    20),
    DEF_BLE_HANDLE("ff12", BLE_HANDLE_FILE_DATA,         BLE_HANDLE_TYPE_INT8_ARR,  20),
    DEF_BLE_HANDLE("ff13", BLE_HANDLE_GPS_INFO,          BLE_HANDLE_TYPE_INT16_ARR, 4),
    DEF_BLE_HANDLE("ff14", BLE_HANDLE_CONF_GESTURE,      BLE_HANDLE_TYPE_INT8_ARR,  5),
    DEF_BLE_HANDLE("ff15", BLE_HANDLE_CONF_WORLDCLOCK_0, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff16", BLE_HANDLE_CONF_WORLDCLOCK_1, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff17", BLE_HANDLE_CONF_WORLDCLOCK_2, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff18", BLE_HANDLE_CONF_WORLDCLOCK_3, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff19", BLE_HANDLE_CONF_WORLDCLOCK_4, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff20", BLE_HANDLE_CONF_WORLDCLOCK_5, BLE_HANDLE_TYPE_STRING,    10 + 4),
    DEF_BLE_HANDLE("ff21", BLE_HANDLE_CONF_WATCHFACE,    BLE_HANDLE_TYPE_INT8_ARR,  1),
    DEF_BLE_HANDLE("ff22", BLE_HANDLE_CONF_GOALS,        BLE_HANDLE_TYPE_INT16_ARR, 3),
    DEF_BLE_HANDLE("ff23", BLE_HANDLE_CONF_USER_PROFILE, BLE_HANDLE_TYPE_INT8_ARR,  3),
    DEF_BLE_HANDLE("ff24", BLE_HANDLE_TODAY_ACTIVITY,    BLE_HANDLE_TYPE_INT32_ARR, 4),
    DEF_BLE_HANDLE("ff25", BLE_HANDLE_CONF_USER_PROFILE, BLE_HANDLE_TYPE_INT8_ARR,  3),
};

uint16_t BLE_READ_INT16(uint8_t* buf)
{
    uint16_t left = buf[1];
    uint16_t right = buf[0];
    return left << 8 | right;
}

void BLE_WRITE_INT16(uint8_t* buf, uint16_t value)
{
    memcpy(buf, &value, 2);
}

uint32_t BLE_READ_INT32(uint8_t* buf)
{
    uint32_t v0 = buf[3];
    uint32_t v1 = buf[2];
    uint32_t v2 = buf[1];
    uint32_t v3 = buf[0];
    return v0 << 24 | v1 << 16 | v2 << 8 | v3;
}

void BLE_WRITE_INT32(uint8_t* buf, uint32_t value)
{
    memcpy(buf, &value, 4);
}

static uint32_t s_sports_data_buffer[5] = {0};
static uint32_t s_sports_desc_buffer[2] = {0};

static void att_read_world_clock(uint8_t id, char* buf, uint8_t buf_size)
{
    ui_config* conf = window_readconfig();
    if (id >= sizeof(conf->worldclock_offset))
        return;

    if (buf_size <= sizeof(conf->worldclock_name[0]) + 3)
        return;

    sprintf(buf, "%s[%d]", conf->worldclock_name[id], conf->worldclock_offset[id]);
    log_info("Read World clock:\n");
    log_info(buf);
    log_info("\n");
}

static void att_write_world_clock(uint8_t id, char* buf)
{
    ui_config* conf = window_readconfig();
    if (id >= sizeof(conf->worldclock_offset))
        return;

    uint8_t i = strlen(buf);
    if (i == 0)
        return;

    uint8_t offset = 0;
    uint8_t shift  = 0;
    for (; i > 0; --i)
    {
        if (buf[i] == ']')
            continue;

        offset = offset * 10 * shift + buf[i] - '0';

        if (buf[i] == '[')
        {
            buf[i] = '\0';
            break;
        }
    }

    conf->worldclock_offset[id] = offset;
    strcpy(conf->worldclock_name[id], buf);

    window_writeconfig();
    window_loadconfig();

}
static void ble_set_alarm(uint8_t id, uint8_t flag, uint8_t hour, uint8_t minutes, uint8_t att_mode)
{
    log_info("Set Alarm()\n");
    if (att_mode == ATT_HANDLE_MODE_WRITE)
    {
        uint8_t alarm_id = id;
        ui_config* conf = window_readconfig();
        conf->alarms[alarm_id].flag    = flag;
        conf->alarms[alarm_id].hour    = hour;
        conf->alarms[alarm_id].minutes = minutes;
        window_writeconfig();
        window_loadconfig();
    }
}

static uint16_t att_handler_internal(uint16_t handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size, uint8_t mode)
{
    const ble_handle_t* hble = get_ble_handle(handle);
    if (hble == NULL)
    {
        log_error("No correspondent handler\n");
        return 0;
    }

    log_info("att_handler(handle=%s, buflen=%d, mode=%d)\n", hble->name, buffer_size, mode);

    uint16_t actual_size =  20; //hble->size * get_type_unit_size(hble->type);
    if (handle == BLE_HANDLE_FILE_DATA && mode == ATT_HANDLE_MODE_WRITE)
        actual_size = 80;
    if (buffer == 0)
    {
        log_error("Invalid Buffer(size=%d/%d)\n", buffer_size, actual_size);
        return actual_size;
    }

    switch (handle)
    {
        case BLE_HANDLE_UNLOCK_WATCH:
            if (mode == ATT_HANDLE_MODE_WRITE)
            {
                system_unlock();
            }
            else
            {
                buffer[0] = 0;
            }
            break;
        case BLE_HANDLE_FW_VERSION:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                strcpy((char*)buffer, FWVERSION);
            }
            break;

        case BLE_HANDLE_SPORTS_GRID:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                ui_config* conf = window_readconfig();
                uint8_t i = 0;
                for (; i < conf->sports_grid + 2 &&
                       i < buffer_size &&
                       i + 1 < sizeof(conf->sports_grid_data); ++i)
                {
                    buffer[i] = conf->sports_grid_data[i + 1];
                }
                for (; i < buffer_size; ++i)
                {
                    buffer[i] = 0xff;
                }
            }
            else
            {
                ui_config* conf = window_readconfig();
                uint8_t i = 0;
                for (; i < buffer_size && i + 1 < sizeof(conf->sports_grid_data); ++i)
                {
                    if (buffer[i] == 0xff)
                    {
                        break;
                    }
                    else
                    {
                        conf->sports_grid_data[i + 1] = buffer[i];
                    }
                }
                if (i < 2) i = 2;
                conf->sports_grid = i - 2;
                window_writeconfig();
                window_loadconfig();
            }
            break;

        case BLE_HANDLE_SPORTS_DATA:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                memcpy(buffer, s_sports_data_buffer, buffer_size);
                log_info("SPORTS_DATA(%d): %ld, %ld, %ld, %ld, %ld\n",
                    buffer_size,
                    s_sports_data_buffer[0],
                    s_sports_data_buffer[1],
                    s_sports_data_buffer[2],
                    s_sports_data_buffer[3],
                    s_sports_data_buffer[4]);
                log_info("SPORTS_DATA[RAW]: %02x %02x %02x %02x, %02x %02x %02x %02x\n",
                    buffer[0], buffer[1], buffer[2], buffer[3],
                    buffer[4], buffer[5], buffer[6], buffer[7]);
            }
            break;

        case BLE_HANDLE_SPORTS_DESC:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                memcpy(buffer, s_sports_desc_buffer, sizeof(s_sports_desc_buffer));
                log_info("SPORTS_DESC[0]=%d\n", buffer[0]);
            }
            break;

        case BLE_HANDLE_DATETIME:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                uint16_t year = 0;
                uint8_t  weekday = 0;
                rtc_readdate(&year, &buffer[1], &buffer[2], &weekday);
                rtc_readtime(&buffer[3], &buffer[4], &buffer[5]);
                buffer[0] = year - 2000;
            }
            else
            {
                handle_clock(buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            }
            break;

        case BLE_HANDLE_ALARM_0:
            ble_set_alarm(0, buffer[0], buffer[1], buffer[2], mode);
            break;
        case BLE_HANDLE_ALARM_1:
            ble_set_alarm(1, buffer[0], buffer[1], buffer[2], mode);
            break;
        case BLE_HANDLE_ALARM_2:
            ble_set_alarm(2, buffer[0], buffer[1], buffer[2], mode);
            break;

        case BLE_HANDLE_DEVICE_ID:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                memset(buffer, 0, buffer_size);
            }
            break;

        case BLE_HANDLE_FILE_DESC:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                log_info("ble_read_file_desc() enter: mode=%x\n", get_file_mode());
                ble_read_file_desc(buffer, buffer_size);
                log_info("ble_read_file_desc() leave: mode=%x, cmd=%c,%d,%d [%x %x]\n", 
                    get_file_mode(), FD_GET_COMMAND(buffer), FD_GET_BLOCKSIZE(buffer), FD_GET_BLOCKID(buffer), buffer[2], buffer[3]);
            }
            else
            {
                log_info("ble_write_file_desc() enter: mode=%x, cmd=%c,%d,%d [%x %x]\n", 
                    get_file_mode(), FD_GET_COMMAND(buffer), FD_GET_BLOCKSIZE(buffer), FD_GET_BLOCKID(buffer), buffer[2], buffer[3]);
                ble_write_file_desc(buffer, buffer_size);
                log_info("ble_write_file_desc() leave: mode=%x\n", get_file_mode());
            }
            break;

        case BLE_HANDLE_FILE_DATA:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                log_info("ble_read_file_data() \n");
                ble_read_file_data(buffer, buffer_size);
            }
            else
            {
                log_info("ble_write_file_data() \n");
                ble_write_file_data(buffer, buffer_size);
            }
            break;

        case BLE_HANDLE_GPS_INFO:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                memset(buffer, 0, buffer_size);
            }
            else
            {
                uint32_t spd  = BLE_READ_INT32(&buffer[0]);
                uint32_t alt  = BLE_READ_INT32(&buffer[4]);
                uint32_t dist = BLE_READ_INT32(&buffer[8]);
                handle_gps_info(spd, alt, dist);
            }
            break;

        case BLE_HANDLE_CONF_GESTURE:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                ui_config* conf = window_readconfig();
                buffer[0] = conf->gesture_flag;
                for (uint8_t i = 0; i < sizeof(conf->gesture_map) && i + 1 < buffer_size; ++i)
                {
                    buffer[i + 1] = conf->gesture_map[i];
                }
            }
            else
            {
                handle_gesture_control(buffer[0], &buffer[1]);
            }
            break;

        case BLE_HANDLE_CONF_WORLDCLOCK_0:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(0, (char*)buffer, buffer_size);
            else
                att_write_world_clock(0, (char*)buffer);
            break;
        case BLE_HANDLE_CONF_WORLDCLOCK_1:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(1, (char*)buffer, buffer_size);
            else
                att_write_world_clock(1, (char*)buffer);
            break;
        case BLE_HANDLE_CONF_WORLDCLOCK_2:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(2, (char*)buffer, buffer_size);
            else
                att_write_world_clock(2, (char*)buffer);
            break;
        case BLE_HANDLE_CONF_WORLDCLOCK_3:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(3, (char*)buffer, buffer_size);
            else
                att_write_world_clock(3, (char*)buffer);
            break;
        case BLE_HANDLE_CONF_WORLDCLOCK_4:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(4, (char*)buffer, buffer_size);
            else
                att_write_world_clock(4, (char*)buffer);
            break;
        case BLE_HANDLE_CONF_WORLDCLOCK_5:
            if (mode == ATT_HANDLE_MODE_READ)
                att_read_world_clock(5, (char*)buffer, buffer_size);
            else
                att_write_world_clock(5, (char*)buffer);
            break;

        case BLE_HANDLE_CONF_WATCHFACE:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                ui_config* conf = window_readconfig();
                buffer[0] = conf->default_clock;
                buffer[1] = conf->default_clock == 0 ? conf->analog_clock : conf->digit_clock;
            }
            else
            {
                ui_config* conf = window_readconfig();
                conf->default_clock = buffer[0];
                if (buffer[0] == 0)
                    conf->analog_clock = buffer[1];
                else
                    conf->digit_clock = buffer[1];
                window_writeconfig();
                window_loadconfig();
            }
            break;

        case BLE_HANDLE_CONF_GOALS:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                ui_config* conf = window_readconfig();
                uint8_t i = 0;
                memcpy(&buffer[i], &conf->goal_steps, sizeof(conf->goal_steps));
                i += sizeof(conf->goal_steps);

                memcpy(&buffer[i], &conf->goal_distance, sizeof(conf->goal_distance));
                i += sizeof(conf->goal_distance);

                memcpy(&buffer[i], &conf->goal_calories, sizeof(conf->goal_calories));
                i += sizeof(conf->goal_calories);
            }
            else
            {
                ui_config* conf = window_readconfig();

                conf->goal_steps    = BLE_READ_INT16(&buffer[0]);
                conf->goal_distance = BLE_READ_INT16(&buffer[2]);
                conf->goal_calories = BLE_READ_INT16(&buffer[4]);

                window_writeconfig();
                window_loadconfig();
            }
            break;

        case BLE_HANDLE_CONF_USER_PROFILE:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                ui_config* conf = window_readconfig();
                buffer[0] = conf->height;
                buffer[1] = conf->weight;
                buffer[2] = conf->circumference;
            }
            else
            {
                ui_config* conf = window_readconfig();

                conf->height        = buffer[0];
                conf->weight        = buffer[1];
                conf->circumference = buffer[2];

                window_writeconfig();
                window_loadconfig();
            }
            break;

        case BLE_HANDLE_TODAY_ACTIVITY:
            if (mode == ATT_HANDLE_MODE_READ)
            {
                BLE_WRITE_INT32(&buffer[0],  (uint32_t)ped_get_time());
                BLE_WRITE_INT32(&buffer[4],  (uint32_t)ped_get_steps());
                BLE_WRITE_INT32(&buffer[8],  (uint32_t)ped_get_calorie());
                BLE_WRITE_INT32(&buffer[12], (uint32_t)ped_get_distance());
            }
            break;
    }

    return actual_size;
}


uint16_t att_handler(uint16_t handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size, uint8_t mode)
{
    if (mode == ATT_HANDLE_MODE_READ)
    {
        return att_handler_internal(handle, offset, buffer, buffer_size, mode);
    }
    else
    {
        if (buffer == NULL)
            return att_handler_internal(handle, offset, buffer, buffer_size, mode);
        else
        {
            att_handler_internal(handle, offset, buffer, buffer_size, mode);
            return 0;
        }
    }
}

uint8_t get_type_unit_size(uint8_t type)
{
    static uint8_t s_uint_size_table[] = {1, 2, 4, 1};
    return s_uint_size_table[type];
}

const ble_handle_t* get_ble_handle(uint16_t handle)
{
    for (uint8_t i = 0; i < sizeof(s_ble_handle_table)/sizeof(s_ble_handle_table[0]); ++i)
    {
        if (s_ble_handle_table[i].handle == handle)
            return &s_ble_handle_table[i];
    }
    return NULL;
}

void ble_start_sync(uint8_t mode, uint8_t sports_type)
{
    s_sports_desc_buffer[0] = mode;
    s_sports_desc_buffer[1] = sports_type;
}

void ble_send_sports_data(uint32_t data[], uint8_t size)
{
    for (uint8_t i = 0; i < size && i < 5; ++i)
        s_sports_data_buffer[i] = data[i];
}

void ble_stop_sync()
{
    s_sports_desc_buffer[0] = 0;
}
