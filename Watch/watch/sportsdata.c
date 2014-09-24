
#include <stdio.h>
#include <string.h>

#include "sportsdata.h"

#include <cfs/cfs.h>
#include "window.h"
#include "btstack/include/btstack/utils.h"
#include "rtc.h"

static const uint32_t signature = 0xEFAB1CC3;
#define MAX_DATA_FILE_COUNT  30

typedef struct _data_desc_t
{
    const char*   file_prefix;
    const uint8_t col_count;
    const uint8_t col_desc[5];
}data_desc_t;

typedef struct _data_file_t
{
    int           file_id;
    uint16_t      row_cursor;
}data_file_t;

/*
 * data here stands for data collected within the time interval
 *  from last row to this one
 *  no Speed data here is due to avg speed can be calculated by distance / timeinterval
 * normal  : steps, cals, distance,
 * running : steps, cals, distance, alt, heartrate
 * biking  : cads,  cals, distance, alt, heartrate*/
/*  Columns are defined as below
    SPORTS_TIME = 0,
    SPORTS_SPEED_MAX,
    SPORTS_TIME_LAST_GPS,

    SPORTS_SPEED,
    SPORTS_ALT,
    SPORTS_DISTANCE,
    SPORTS_ALT_START,

    SPORTS_HEARTRATE,
    SPORTS_HEARTRATE_AVG,
    SPORTS_HEARTRATE_MAX,
    SPORTS_TIME_LAST_HEARTRATE,

    SPORTS_CADENCE,
    SPORTS_TIME_LAP_BEGIN,
    SPORTS_LAP_BEST,

    SPORTS_STEPS,
    SPORTS_PED_SPEED,
    SPORTS_PED_DISTANCE,
    SPORTS_PED_CALORIES,
    SPORTS_PED_STEPS_START,
    SPORTS_TIME_LAST_PED,

    SPORTS_CALS,
static const data_desc_t s_data_desc[] = {
    {"w", 3, {SPORTS_TIME, SPORTS_STEPS,   SPORTS_CALS, SPORTS_DISTANCE, SPORTS_INVALID},  },
    {"r", 4, {SPORTS_TIME, SPORTS_STEPS,   SPORTS_CALS, SPORTS_DISTANCE, SPORTS_HEARTRATE},},
    {"b", 4, {SPORTS_TIME, SPORTS_CADENCE, SPORTS_CALS, SPORTS_DISTANCE, SPORTS_HEARTRATE},},
};

*/
typedef struct _data_head_t
{
    uint8_t version;
    uint8_t year;
    uint8_t month;
    uint8_t day;
}data_head_t;

static int check_file_format(int fd)
{
    uint32_t signature = 0;
    data_head_t head;
    uint8_t rowhead[8];
    uint32_t rowdata[8];
    int size = 0;

    size = cfs_read(fd, &signature, sizeof(signature));
    if (size != sizeof(signature))
    {
        printf("check_file_format() signature error: size = %d/%d\n", size, (int)sizeof(signature));
        cfs_close(fd);
        return -1;
    }

    size = cfs_read(fd, &head, sizeof(head));
    if (size != sizeof(head))
    {
        printf("check_file_format() head error\n");
        cfs_close(fd);
        return -1;
    }

    for(;;)
    {
        size = cfs_read(fd, &rowhead, sizeof(rowhead));
        if (size == 0)
            return fd; //read nothing means the file is well formatted

        if (size == sizeof(rowhead))
        {
            if (rowhead[3] > 8)
            {
                printf("check_file_format() row head error\n");
                cfs_close(fd);
                return -1;
            }

            int data_size = rowhead[3] * sizeof(rowdata[0]);
            size = cfs_read(fd, &rowdata, data_size);
            if (size == data_size)
                continue;

            if (size < data_size)
            {
                //padding 0
                uint32_t pad[8] = {0};
                cfs_write(fd, pad, data_size - size);
                return fd;
            }
            else
            {
                printf("check_file_format() row data read error\n");
                cfs_close(fd);
                return -1;
            }

        }

        if (size < sizeof(rowhead))
        {
            uint8_t pad[8] = {0};
            cfs_write(fd, pad, sizeof(rowhead) - size);
            return fd;
        }

        if (size > sizeof(rowhead))
        {
            printf("check_file_format() row head read error\n");
            cfs_close(fd);
            return -1;
        }
    }

}

/*
static uint8_t is_today_file(uint8_t year, uint8_t month, uint8_t day)
{
    uint16_t cyear;
    uint8_t cmonth, cday, dweekday;
    rtc_readdate(&cyear, &cmonth, &cday, &dweekday);

    return cday == day && cmonth == month && cyear == year;
}
*/

static uint8_t is_data_file(char* name)
{
    if (name[0] == '/' &&
        name[1] == 'D' &&
        name[2] == 'A' &&
        name[3] == 'T' &&
        name[4] == 'A' &&
        name[5] == '/')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint8_t check_file_name(char* name, uint8_t* year, uint8_t* month, uint8_t* day)
{

    if (is_data_file(name))
    {
        if (name[8] != '-' || name[11] != '-')
        {
            return 0;
        }

        *year  = (name[6]  - '0') * 10 + (name[7]  - '0');
        *month = (name[9]  - '0') * 10 + (name[10] - '0');
        *day   = (name[12] - '0') * 10 + (name[13] - '0');
        if (*year >= 100 || *month > 12 || *day > 31)
        {
            return 0;
        }

        return 1;
    }

    return 0;
}

static void write_file_head(int fd, uint8_t year, uint8_t month, uint8_t day)
{
    cfs_write(fd, &signature, sizeof(signature));

    data_head_t data_head;
    data_head.version = 1;
    data_head.year    = year;
    data_head.month   = month;
    data_head.day     = day;
    cfs_write(fd, &data_head, sizeof(data_head));
}

static int s_data_fd = -1;
static const char* s_data_dir = "DATA";

uint8_t is_active_data_file(char* filename)
{
    uint8_t fyear, fmonth, fday;
    if (!check_file_name(filename, &fyear, &fmonth, &fday))
    {
        return 0;
    }

    uint16_t year;
    uint8_t month, day, weekday;
    rtc_readdate(&year, &month, &day, &weekday);
    return (year % 100) == fyear && month == fmonth && day == fday;
}

int create_data_file(uint8_t year, uint8_t month, uint8_t day)
{
    char filename[32] = "";
    sprintf(filename, "/%s/%02d-%02d-%02d", s_data_dir, year, month, day);

    uint8_t fyear, fmonth, fday;
    if (!check_file_name(filename, &fyear, &fmonth, &fday))
    {
        return -1;
    }

    int fd = cfs_open(filename, CFS_READ | CFS_WRITE);
    if (fd != -1)
    {
        if (check_file_format(fd) != fd)
        {
            printf("Remove file\n");
            cfs_remove(filename);
        }
        else
        {
            s_data_fd = fd;
            return fd;
        }
    }

    s_data_fd = cfs_open(filename, CFS_READ | CFS_WRITE);
    if (s_data_fd == -1)
    {
        printf("create_data_file(%d, %d, %d) failed\n", year, month, day);
    }
    else
    {
        printf("create_data_file(%d, %d, %d) ok\n", year, month, day);
        write_file_head(s_data_fd, year, month, day);
    }
    return s_data_fd;
}

uint8_t build_data_line(
    uint8_t* buf,
    uint8_t buf_size,
    uint8_t mode, 
    uint8_t hh, uint8_t mm, 
    const uint8_t* meta,
    uint32_t* data,
    uint8_t size)
{
    uint8_t pos = 0;

    //build tag
    buf[pos++] = mode;
    buf[pos++] = hh;
    buf[pos++] = mm;
    buf[pos++] = size;
    if (pos >= buf_size)
        return 0;

    //build meta
    pos += build_data_schema(&buf[pos], meta, size);
    if (pos >= buf_size)
        return 0;

    //build data
    memcpy(&buf[pos], data, size * sizeof(data[0]));
    pos += size * sizeof(data[0]);
    if (pos >= buf_size)
        return 0;

    return pos;
}

void write_data_line(uint8_t mode, uint8_t hh, uint8_t mm, const uint8_t* meta, uint32_t* data, uint8_t size)
{
    if (s_data_fd == -1)
    {
        uint16_t year = 0;
        uint8_t month, day, weekday;
        rtc_readdate(&year, &month, &day, &weekday);
        create_data_file(year % 100, month, day);
    }

    if (s_data_fd != -1)
    {
        uint8_t buf[4 + 4 + 4 * 8] = {0};
        uint8_t buf_size = build_data_line(buf, sizeof(buf), mode, hh, mm, meta, data, size);
        if (buf_size == 0)
        {
            printf("build_data_line(%d, %x, %d, %d, %d) failed\n", s_data_fd, mode, hh, mm, size);
            return;
        }

        if (cfs_write(s_data_fd, buf, buf_size) != buf_size)
        {
            printf("write_data(%d, %x, %d, %d, %d) failed\n", s_data_fd, mode, hh, mm, buf_size);
            close_data_file();
            return;
        }
    }

}

void close_data_file()
{
    if (s_data_fd != -1)
    {
        cfs_close(s_data_fd);
        s_data_fd = -1;
    }
}


void clear_data_file()
{

    struct cfs_dir dir;
    int ret = cfs_opendir(&dir, "");
    if (ret == -1)
    {
        printf("cfs_opendir() failed: %d\n", ret);
    }

    uint16_t min_data_hash = 0;
    char min_data_file[32] = "";
    uint8_t file_count = 0;
    while (ret != -1)
    {
        struct cfs_dirent dirent;
        ret = cfs_readdir(&dir, &dirent);
        if (ret != -1)
        {
            if (!is_data_file(dirent.name))
                continue;

            uint8_t year, month, day;
            if (!check_file_name(dirent.name, &year, &month, &day))
            {
                cfs_remove(dirent.name);
                continue;
            }

            ++file_count;

            uint16_t hash = year * 366 + month * 12 + day;
            if (min_data_hash == 0 || min_data_hash > hash)
            {
                min_data_hash = hash;
                strcpy(min_data_file, dirent.name);
            }
        }
    }
    cfs_closedir(&dir);

    if (file_count > MAX_DATA_FILE_COUNT && min_data_hash != 0)
    {
        cfs_remove(min_data_file);
    }

}

char* get_data_file(uint32_t* filesize)
{
    struct cfs_dir dir;
    int ret = cfs_opendir(&dir, "");
    if (ret == -1)
    {
        printf("cfs_opendir() failed: %d\n", ret);
        return 0;
    }

    static char filename[20] = "";
    uint8_t found = 0;
    while (ret != -1)
    {
        struct cfs_dirent dirent;
        ret = cfs_readdir(&dir, &dirent);
        if (ret != -1)
        {
            uint8_t year, month, day;
            if (!check_file_name(dirent.name, &year, &month, &day))
                continue;

            printf("get_data_file():%s, %d\n", dirent.name, (uint16_t)dirent.size);
            found = 1;
            strcpy(filename, dirent.name);
            *filesize = dirent.size;

            if (!is_active_data_file(dirent.name))
                break;
        }
    }

    cfs_closedir(&dir);

    if (found)
    {
        return filename;
    }
    else
    {
        return 0;
    }

}

void remove_data_file(char* filename)
{
    cfs_remove(filename);
}

uint8_t build_data_schema(uint8_t* buf, const uint8_t* coltype, uint8_t colcount)
{
    uint8_t pos = 0;
    for (uint8_t i = 0; i < colcount; ++i)
    {
        uint8_t val = coltype[i] & 0x0f;
        if ((i & 0x01) == 0)
        {
            buf[pos] = val << 4;
        }
        else
        {
            buf[pos] |= val;
            ++pos;
        }
    }
    return 4;
}


static uint8_t s_cur_mode = DATA_MODE_NORMAL;
uint8_t set_mode(uint8_t mode)
{
    uint8_t oldmode = s_cur_mode;
    s_cur_mode = mode;
    return oldmode;
}

uint8_t get_mode()
{
    return s_cur_mode;
}
