
#include "CuTest.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "TestUtility/stlv_test_stub.h"

#include "ble_handler.h"
#include "watch/sportsdata.h"
#include "cfs/cfs-coffee.h"

#define BLE_HANDLE_SEED_BEGIN 0x000b
#define BLE_HANDLE_SEED_END   0x0037

#define FILE_SIZE (8 * 1024 - 1)
#define FILE_NAME "/DATA/14-03-03"
static uint8_t s_test_buffer_in[20] = {0};
static uint8_t s_test_buffer_out[20] = {0};

static void dumpBuffer(uint8_t* buf, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (i > 0 && i % 16 == 0)
            printf("\n");
        printf("%02x ", buf[i]);
    }
    if (size > 0)
        printf("\n");
}

int TestReadBleFile(CuTest* tc, char* filename, int unitsize)
{
    //write command "reset"
    printf("write FILE_DESC:X\n"); 
    s_test_buffer_in[0] = 'X';
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);
        //read process
    uint8_t read_block_id = 0;
    uint16_t read_block_size = 0;
    int read_cursor = 0;

    //write command "Investigation"
    printf("write FILE_DESC:I\n"); 
    s_test_buffer_in[0] = 'I';
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

    //read back desc
    printf("read FILE_DESC:\n");
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
    dumpBuffer(s_test_buffer_out, sizeof(s_test_buffer_out));

    CuAssertIntEquals(tc, 'F', s_test_buffer_out[0]);
    CuAssertStrEquals(tc, filename, (char*)&s_test_buffer_out[4]);

    while (1)
    {
        printf("read_block_id=%d, block_size=%d\n", read_block_id, read_block_size);

        //write command "Read"
        printf("write FILE_DESC:R, %d, %d\n", 0, read_block_id);
        memcpy(s_test_buffer_in, s_test_buffer_out, 20);
        s_test_buffer_in[0] = 'R';
        s_test_buffer_in[1] = read_block_id;
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

        //read back desc
        printf("read FILE_DESC:\n");
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
        dumpBuffer(s_test_buffer_out, sizeof(s_test_buffer_out));

        if (s_test_buffer_out[0] == 'D')
        {
            read_block_id = s_test_buffer_out[1];
            read_block_size = *((uint16_t*)(&s_test_buffer_out[2]));
            printf("blockid=%d, size=%d\n", read_block_id, read_block_size);

            CuAssertIntEquals(tc, read_block_id, s_test_buffer_out[1]);

            //read back data
            int readpos = 0;
            while (readpos < read_block_size)
            {
                printf("read FILE_DATA:\n");
                att_handler(BLE_HANDLE_FILE_DATA, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
                dumpBuffer(s_test_buffer_out, sizeof(s_test_buffer_out));
                readpos += 20;
            }

            read_block_id++;
        }
        else if (s_test_buffer_out[0] == 'E')
        {
            return read_block_id;
        }
        else if (s_test_buffer_out[0] == 'O')
        {
            read_block_id++;
            break;
        }
        else
        {
            CuAssertTrue(tc, 0);
            return -1;
        }
    }

    return -1;
}

static void TestBleFile(CuTest* tc)
{
    cfs_coffee_format();

    //write process
    printf("Write('W')\n");
    s_test_buffer_in[0] = 'W';
    sprintf((char*)&s_test_buffer_in[1], FILE_NAME);
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
    CuAssertIntEquals(tc, 'H', s_test_buffer_out[0]);

    uint16_t blockid = 0;
    while (blockid < 20)
    {
        printf("Write('S')\n");
        //write desc
        s_test_buffer_in[0] = 'S';
        {
            //block id
            s_test_buffer_in[1] = blockid;
            uint16_t pos = blockid * 20;
            if (FILE_SIZE < pos)
                break;

            //size
            uint16_t sizeleft = FILE_SIZE - pos;
            if (sizeleft > 19)
                sizeleft = 19;
            *((uint16_t*)&s_test_buffer_in[2]) = sizeleft;

            //file name
            sprintf((char*)&s_test_buffer_in[4], FILE_NAME);
        }
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

        //read back desc
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
        CuAssertIntEquals(tc, 'P', s_test_buffer_out[0]);

        printf("WriteData(%d)\n", blockid);
        //write data
        for (int i = 1; i < sizeof(s_test_buffer_in); ++i)
            s_test_buffer_in[i] = (uint8_t)('A' + blockid);
        s_test_buffer_in[0] = 0;
        att_handler(BLE_HANDLE_FILE_DATA, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

        //read back desc
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
        CuAssertIntEquals(tc, 'O', s_test_buffer_out[0]);

        //next block
        blockid++;
    }
    //write command "Investigation"
    s_test_buffer_in[0] = 'C';
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

    TestReadBleFile(tc, FILE_NAME, 0);
}

static void TestBleSportsDataFile(CuTest* tc)
{
    cfs_coffee_format();
    init_send_pack_stub(); 

    uint8_t meta[] = {DATA_COL_STEP, DATA_COL_DIST, DATA_COL_HR};
    uint32_t data[] = {1234, 5678, 9012};
    create_data_file(12, 7, 9);
    write_data_line(0x00, 1, 1, meta, data, 3);
    write_data_line(0x00, 1, 2, meta, data, 3);
    write_data_line(0x01, 1, 3, meta, data, 3);
    write_data_line(0x02, 1, 4, meta, data, 3);
    write_data_line(0x03, 1, 5, meta, data, 3);
    close_data_file();

    int ret = TestReadBleFile(tc, "/DATA/12-07-09", 0);
    CuAssertIntEquals(tc, 1, ret);
}

static void TestBleFile2(CuTest* tc)
{
    cfs_coffee_format();

    //write process
    printf("Write('W')\n");
    s_test_buffer_in[0] = 'W';
    sprintf((char*)&s_test_buffer_in[1], FILE_NAME);
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
    CuAssertIntEquals(tc, 'H', s_test_buffer_out[0]);

    uint16_t blockid = 0;
    while (blockid < 10)
    {
        printf("Write('S')\n");
        uint16_t sizeleft = 0;
        //write desc
        s_test_buffer_in[0] = 'S';
        {
            //block id
            s_test_buffer_in[1] = blockid;
            uint16_t pos = blockid * 20;
            if (FILE_SIZE < pos)
                break;

            //size
            sizeleft = FILE_SIZE - pos;
            if (sizeleft > 1200)
                sizeleft = 1200;
            *((uint16_t*)&s_test_buffer_in[2]) = sizeleft;

            //file name
            sprintf((char*)&s_test_buffer_in[4], FILE_NAME);
        }
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

        //read back desc
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
        CuAssertIntEquals(tc, 'P', s_test_buffer_out[0]);

        printf("WriteData(%d)\n", blockid);
        //write data
        for (int i = 1; i < sizeof(s_test_buffer_in); ++i)
            s_test_buffer_in[i] = (uint8_t)('A' + blockid);

        int subblockid = 0;
        for (int i = 0; i < sizeleft; i += 19)
        {
            s_test_buffer_in[0] = subblockid;
            att_handler(BLE_HANDLE_FILE_DATA, 0, 
                s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);
            subblockid++;
        }

        //read back desc
        att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_out, sizeof(s_test_buffer_out), ATT_HANDLE_MODE_READ);
        CuAssertIntEquals(tc, 'O', s_test_buffer_out[0]);

        //next block
        blockid++;
    }
    //write command "Investigation"
    s_test_buffer_in[0] = 'C';
    att_handler(BLE_HANDLE_FILE_DESC, 0, s_test_buffer_in, sizeof(s_test_buffer_in), ATT_HANDLE_MODE_WRITE);

    TestReadBleFile(tc, FILE_NAME, 0);
}

static void TestBleSportsData(CuTest* tc)
{
    uint32_t sample[] = {123456789, 987654321, 123454321, 987656789};
    uint8_t buffer[20] = {0};
    for (int i = 0; i < 10; ++i)
    {
        printf("turn %d\n", i);
        ble_start_sync(i, 0);
        att_handler(BLE_HANDLE_SPORTS_DESC, 0, buffer, sizeof(buffer), ATT_HANDLE_MODE_READ);
        CuAssertIntEquals(tc, i, buffer[0]);

        ble_send_sports_data(sample, 4);
        att_handler(BLE_HANDLE_SPORTS_DATA, 0, buffer, sizeof(buffer), ATT_HANDLE_MODE_READ);

        uint32_t* p = (uint32_t*)buffer;

        for (int j = 0; j < 4; ++j)
        {
            CuAssertIntEquals(tc, sample[j], p[j]);
        }
    }
}

static void TestBleUnlock(CuTest* tc)
{
    uint8_t buffer[20] = {0};
    att_handler(BLE_HANDLE_UNLOCK_WATCH, 0, buffer, sizeof(buffer), ATT_HANDLE_MODE_WRITE);
}

static void TestBleFirmwareVersion(CuTest* tc)
{
    uint8_t buffer[20] = {0};
    att_handler(BLE_HANDLE_FW_VERSION, 0, buffer, sizeof(buffer), ATT_HANDLE_MODE_READ);
    CuAssertStrEquals(tc, "DEBUG", (char*)buffer);
}

CuSuite* BleHandlerTestGetSuite(void)
{
    CuSuite* suite = CuSuiteNew("ble");
    SUITE_ADD_TEST(suite, TestBleFile);
    SUITE_ADD_TEST(suite, TestBleSportsDataFile);
    SUITE_ADD_TEST(suite, TestBleFile2);
    SUITE_ADD_TEST(suite, TestBleSportsData);
    SUITE_ADD_TEST(suite, TestBleUnlock);
    SUITE_ADD_TEST(suite, TestBleFirmwareVersion);
    return suite;
}

