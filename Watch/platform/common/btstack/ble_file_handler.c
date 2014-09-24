
#include "ble_file_handler.h"

#include <stdio.h>
#include <string.h>

#include "btstack-config.h"
#include "debug.h"

#include "watch/sportsdata.h"
#include "cfs/cfs.h"
#include "stlv_handler.h"

//reset
#define FD_REST           'X'

//for send firmware
#define FD_INVESTIGATE    'I'
#define FD_FILE_FOUND     'F'
#define FD_NO_FILE        'N'
#define FD_READ_FILE      'R'
#define FD_DATA_TRAN      'D'
#define FD_END_OF_DATA    'E'
#define FD_ERROR_RESEND   'Y'

//for read data file
#define FD_WRITE_FILE     'W'
#define FD_WRITE_HANDLED  'H'
#define FD_SEND_DATA      'S'
#define FD_BLOCK_PREPARED 'P'
#define FD_BLOCK_OVER     'O'
#define FD_SEND_COMPLETE  'C'

//file transferring status
#define FS_IDLE          0x00
#define FS_INVESTIGATING 0x01
#define FS_READING       0x02
#define FS_NO_DATA       0x03
#define FS_FILE_FOUND    0x04
#define FS_TRANSFERRIG   0x05

#define FS_WRITE         0x10
#define FS_WF_PREPARED   0x20
#define FS_SENDING       0x30
#define FS_SEND_OK       0x40
#define FS_SEND_ERR      0x50

#define READ_BLOCK_SIZE  300

static uint8_t  s_file_mode        = FS_IDLE;
static uint8_t  s_file_desc[20]    = {0};
static char     s_file_name[20]    = "";

static int      s_read_fd          = -1;
static int      s_write_fd         = -1;
static uint32_t s_file_size        = 0;
static uint32_t s_file_cursor      = 0;
static uint32_t s_this_block_size  = 0;
static uint8_t  s_file_data[20]    = "";
static uint8_t  s_block_id         = 0xff;
static uint8_t  s_sub_block        = 0;
static uint16_t s_sub_block_offset = 0;
static uint16_t s_block_cursor     = 0;


//FILE_DESC parse utility
#define FD_GET_COMMAND(buf)   (buf[0])
#define FD_GET_BLOCKID(buf)   (buf[1])
#define FD_GET_FILENAME(buf)  ((char*)&buf[4])

#define FD_SET_COMMAND(buf, cmd)        buf[0] = cmd
#define FD_SET_BLOCKID(buf, blockid)    buf[1] = blockid
#define FD_SET_FILENAME(buf, filename)  strcpy((char*)&buf[4], filename)

uint16_t FD_GET_BLOCKSIZE(uint8_t* buf)
{
    uint16_t left = buf[3];
    uint16_t right = buf[2];
    return left << 8 | right;
}

void FD_SET_BLOCKSIZE(uint8_t* buf, uint16_t blocksize)
{
    memcpy(&buf[2], &blocksize, 2);
}

uint8_t get_file_mode()
{
    return s_file_mode;
}

static void init_ble_file_handler()
{
    s_file_mode = FS_IDLE;
    memset(s_file_desc, 0, sizeof(s_file_desc));
    memset(s_file_data, 0, sizeof(s_file_desc));

    s_block_id = 0xff;
    if (s_read_fd != -1)
    {
        cfs_close(s_read_fd);
        s_read_fd = -1;
    }
}

//functionaility
void ble_write_file_desc(uint8_t* buffer, uint16_t buffer_size)
{

    if (FD_GET_COMMAND(buffer) == FD_REST)
    {
        init_ble_file_handler();
        return;
    }

    switch(s_file_mode)
    {
        case FS_INVESTIGATING:
        break;

        case FS_IDLE:
        if (FD_GET_COMMAND(buffer) == FD_INVESTIGATE)
        {
            s_file_mode = FS_INVESTIGATING;
            return;
        }

        if (FD_GET_COMMAND(buffer) == FD_WRITE_FILE)
        {
            s_file_mode = FS_WRITE;
            return;
        }
        break;

        case FS_WRITE:
        if (FD_GET_COMMAND(buffer) == FD_SEND_DATA)
        {
            memcpy(s_file_desc, buffer, buffer_size);
            s_file_mode = FS_WF_PREPARED;
            s_block_id = 0;
            return;
        }
        break;

        case FS_WF_PREPARED:
        if (FD_GET_COMMAND(buffer) == FD_SEND_DATA)
        {
            char* newfilename = FD_GET_FILENAME(buffer);
            char* oldfilename = FD_GET_FILENAME(s_file_desc);

            uint8_t newblockid = FD_GET_BLOCKID(buffer);
            uint8_t oldblockid = FD_GET_BLOCKID(s_file_desc);

            if (strcmp(newfilename, oldfilename) != 0 || newblockid != oldblockid)
                init_ble_file_handler();
            else
                FD_SET_COMMAND(s_file_desc, FD_SEND_DATA);
            return;
        }
        break;

        case FS_SENDING:
        if (FD_GET_COMMAND(buffer) == FD_SEND_DATA)
        {
            memcpy(s_file_desc, buffer, sizeof(s_file_desc));
            s_file_mode = FS_WF_PREPARED;
            return;
        }

        if (FD_GET_COMMAND(buffer) == FD_SEND_COMPLETE)
        {
            handle_file_end(s_write_fd);
            s_write_fd = -1;

            init_ble_file_handler();
            s_file_mode = FS_IDLE;
            return;
        }
        break;

        case FS_SEND_ERR:
        if (FD_GET_COMMAND(buffer) == FD_SEND_DATA &&
            FD_GET_BLOCKID(buffer) == s_block_id)
        {
            memcpy(s_file_desc, buffer, sizeof(s_file_desc));
            s_file_mode = FS_WF_PREPARED;
            return;
        }
        break;

        case FS_FILE_FOUND:
        if (FD_GET_COMMAND(buffer) == FD_READ_FILE)
        {
            memcpy(s_file_desc, buffer, sizeof(s_file_desc));
            s_file_mode = FS_TRANSFERRIG;
            return;
        }
        break;

        case FS_READING:
        if (FD_GET_COMMAND(buffer) == FD_READ_FILE)
        {
            memcpy(s_file_desc, buffer, sizeof(s_file_desc));
            s_file_mode = FS_TRANSFERRIG;
            return;
        }
        break;
    }

}

void ble_read_file_desc(uint8_t * buffer, uint16_t buffer_size)
{
    
    switch (s_file_mode)
    {
        case FS_IDLE:
        case FS_NO_DATA:
        case FS_FILE_FOUND:
            //do nothing
            break;

        // for data upload
        case FS_INVESTIGATING:
        {
            char* name = get_data_file(&s_file_size);
            s_file_cursor      = 0;
            s_sub_block_offset = 0;
            if (name == NULL)
            {
                FD_SET_COMMAND(s_file_desc, FD_NO_FILE);
                s_file_mode = FS_IDLE;
            }
            else
            {
                s_this_block_size = s_file_size - s_file_cursor;
                if (s_this_block_size > READ_BLOCK_SIZE)
                    s_this_block_size = READ_BLOCK_SIZE;
                FD_SET_COMMAND(s_file_desc, FD_FILE_FOUND);
                FD_SET_BLOCKID(s_file_desc, 0);
                FD_SET_BLOCKSIZE(s_file_desc, s_this_block_size);
                FD_SET_FILENAME(s_file_desc, name);
                s_file_mode = FS_FILE_FOUND;
                s_block_id = 0;
            }
        }
        break;

        case FS_READING:
        case FS_TRANSFERRIG:
            FD_SET_COMMAND(s_file_desc, FD_DATA_TRAN);
            FD_SET_BLOCKSIZE(s_file_desc, s_this_block_size);
            break;

        //for fw upgrade
        case FS_WRITE:
            FD_SET_COMMAND(s_file_desc, FD_WRITE_HANDLED);
            break;

        case FS_WF_PREPARED:
            FD_SET_COMMAND(s_file_desc, FD_BLOCK_PREPARED);
            break;

        case FS_SENDING:
            FD_SET_COMMAND(s_file_desc, FD_BLOCK_OVER);
            break;

        case FS_SEND_ERR:
            FD_SET_COMMAND(s_file_desc, FD_ERROR_RESEND);
            break;

    }

    memcpy(buffer, s_file_desc, buffer_size);
    
}

void ble_read_file_data(uint8_t* buffer, uint8_t buffer_size)
{
    if (s_file_mode == FS_TRANSFERRIG)
    {
        uint8_t blockid = FD_GET_BLOCKID(s_file_desc);
        if (blockid != s_block_id)
        {
            log_error("wrong block id: exp %d, act %d\n", s_block_id, blockid);
            FD_SET_COMMAND(s_file_desc, FD_END_OF_DATA);
                s_file_mode = FS_IDLE;
                return;
        }

        if (s_read_fd == -1)
        {
            char* filename = FD_GET_FILENAME(s_file_desc);
            s_read_fd = cfs_open(filename, CFS_READ);
            if (s_read_fd == -1)
            {
                log_error("cfs_open(%s) failed when FS_READING\n", filename);
                FD_SET_COMMAND(s_file_desc, FD_END_OF_DATA);
                s_file_mode = FS_IDLE;
                return;
            }
            strcpy(s_file_name, filename);
        }

        int read_byte = cfs_read(s_read_fd, buffer, buffer_size);
        if (read_byte == 0)
        {
            log_info("file sent complete\n");
            s_file_size       = 0;
            s_file_cursor     = 0;
            s_this_block_size = 0;
            s_block_id        = 0;

            cfs_close(s_read_fd);
            s_read_fd = -1;

            if (!is_active_data_file(s_file_name))
            {
                remove_data_file(s_file_name);
            }

            FD_SET_COMMAND(s_file_desc, FD_END_OF_DATA);
            s_file_mode = FS_IDLE;
            return;
        }
        s_block_cursor += read_byte;

        if (s_block_cursor >= READ_BLOCK_SIZE)
        {
            s_file_cursor += s_block_cursor;
            s_block_cursor = 0;

            if (s_file_size <= s_file_cursor)
            {
                log_info("file sent complete\n");
                s_file_size       = 0;
                s_file_cursor     = 0;
                s_this_block_size = 0;
                s_block_id        = 0;

                cfs_close(s_read_fd);
                s_read_fd = -1;

                FD_SET_COMMAND(s_file_desc, FD_END_OF_DATA);
                s_file_mode = FS_IDLE;
                return;
            }

            s_this_block_size = s_file_size - s_file_cursor;
            if (s_this_block_size > READ_BLOCK_SIZE)
                s_this_block_size = READ_BLOCK_SIZE;

            log_info("send next block id=%d, size=%d\n", s_block_id, s_this_block_size);
            s_block_id = blockid + 1;
            FD_SET_BLOCKSIZE(s_file_desc, s_this_block_size);
            FD_SET_COMMAND(s_file_desc, FD_DATA_TRAN);

            s_file_mode = FS_READING;
        }
    }
    else
    {
        memset(buffer, 0, buffer_size);
    }
}

void ble_write_file_data(uint8_t* buffer, uint8_t buffer_size)
{
    if (s_file_mode != FS_WF_PREPARED)
        return;

    uint8_t blockid = FD_GET_BLOCKID(s_file_desc);
    if (s_write_fd == -1)
    {
        char* filename = FD_GET_FILENAME(s_file_desc);
        s_write_fd = handle_file_begin(filename);
        if (s_write_fd == -1)
        {
            log_error("handle_file_begin(%s) failed\n", filename);
            return;
        }
    }

    if (blockid > s_block_id + 1)
    {
        log_error("wrong block id:%d - %d\n", blockid, s_block_id);
        init_ble_file_handler();
        return;
    }
    else if (blockid < s_block_id)
    {
        log_error("wrong block id:%d - %d\n", blockid, s_block_id);
        return;
    }
    else
    {
        uint16_t block_size = FD_GET_BLOCKSIZE(s_file_desc);
        if (block_size < s_sub_block_offset)
        {
            log_error("wrong block size:%d - %d\n", block_size, s_sub_block_offset);
            init_ble_file_handler();
            return;
        }
        uint16_t sub_block_size = block_size - s_sub_block_offset;
        uint8_t sub_block_id = buffer[0];
        if (sub_block_id != s_sub_block)
        {
            log_error("sub block id error: exp-%x, act-%x\n", s_sub_block, sub_block_id);
            s_file_mode = FD_ERROR_RESEND;
            return;
        }

        if (sub_block_size > buffer_size - 1)
            sub_block_size = buffer_size - 1;

        log_info("Recv:sub block:sid=%u,size=%d\n", sub_block_id, sub_block_size);
        handle_file_data(s_write_fd, &buffer[1], sub_block_size);

        s_sub_block_offset += sub_block_size;
        s_sub_block++;

        if (s_sub_block_offset == block_size)
        {
            s_block_id = blockid + 1;
            s_sub_block_offset = 0;
            s_sub_block = 0;
            s_file_mode = FS_SENDING;
            log_info("whole block(%d,%d) receved\n", s_block_id, block_size);
        }

    }

}
