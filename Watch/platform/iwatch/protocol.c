#include "contiki.h"
#include <stdio.h>
#include "uart1.h"
#include <cfs/cfs.h>
#include <cfs/cfs-coffee.h>

PROCESS(protocol_process, "Protocol Handle");

/********************** Protocol Definition *************************/
// ret code
#define RX_PACKET_ONGOING 0x00
#define DATA_RECEIVED 0x01
#define RX_ERROR_RECOVERABLE 0x02
#define RX_ERROR_REINIT 0x03
#define RX_ERROR_FATAL 0x04
#define PI_DATA_RECEIVED 0x05

#define PI_COMMAND_UPPER 0x50

//errors
#define HEADER_INCORRECT (PI_COMMAND_UPPER + 0x01)
#define CHECKSUM_INCORRECT (PI_COMMAND_UPPER + 0x02)
#define PACKET_SIZE_ZERO (PI_COMMAND_UPPER + 0x03)
#define PACKET_SIZE_TOO_BIG (PI_COMMAND_UPPER + 0x04)
#define UNKNOWN_ERROR (PI_COMMAND_UPPER + 0x05)
// errors for PI commands
#define UNKNOWN_BAUD_RATE (PI_COMMAND_UPPER + 0x06)

// TA PI Commands
#define CHANGE_BAUD_RATE (PI_COMMAND_UPPER + 0x02)

#define MAX_BUFFER_SIZE 260

//Received commands
#define RX_DATA_BLOCK      0x10
#define RX_PASSWORD        0x11
#define ERASE_SEGMENT      0x12
#define TOGGLE_INFO        0x13
#define ERASE_BLOCK        0x14
#define MASS_ERASE         0x15
#define CRC_CHECK          0x16
#define LOAD_PC            0x17
#define TX_DATA_BLOCK      0x18
#define TX_BSL_VERSION     0x19
#define TX_BUFFER_SIZE     0x1A
#define RX_DATA_BLOCK_FAST 0x1B

#define TX_FILE_BEGIN      0x30
#define TX_FILE_END        0x31
#define TX_FILE_REMOVE     0x34

#define TX_LOG_GET         0x35

//Responses
#define BSL_DATA_REPLY    0x3A
#define BSL_MESSAGE_REPLY 0x3B
#define ACK               SUCCESSFUL_OPERATION

// Error Codes
// - From the API
#define SUCCESSFUL_OPERATION 0x00
#define NO_EXCEPTIONS 0x00
#define MEMORY_WRITE_CHECK_FAILED 0x01
#define FLASH_FAIL_BIT_SET 0x02
#define VOLTAGE_CHANGE_DURING_PROGRAM 0x03
#define BSL_LOCKED 0x04
#define BSL_PASSWORD_ERROR 0x05
#define BYTE_WRITE_FORBIDDEN 0x06
// - From the Command Interpreter
#define UNKNOWN_COMMAND 0x07
#define LENGTH_TOO_BIG_FOR_BUFFER 0x08

#define sendByte(data) uart_sendByte(data)
#define getByte() uart_getByte()

static void PI_sendData(int size);

static char *BSL430_ReceiveBuffer;
static char *BSL430_SendBuffer;
static unsigned int BSL430_BufferSize;

static char RAM_Buf[MAX_BUFFER_SIZE];
static char Log_Buf[160];
static uint8_t LogWrite;

void protocol_init()
{
    BSL430_ReceiveBuffer = RAM_Buf;
    BSL430_SendBuffer = RAM_Buf;

    LogWrite = 1;
}

void protocol_start(uint8_t start)
{
	if (start)
		process_start(&protocol_process, NULL);
	else
		process_exit(&protocol_process);
}

int putchar(int data)
{
#if 0
    int x = splhigh();
    Log_Buf[LogWrite++] = data;
    if (LogWrite >= sizeof(Log_Buf)) LogWrite = 0;
    //if (LogRead == LogWrite) LogRead = LogWrite;
    splx(x);
#else
    uart_sendByte(data);
#endif
    
    return data;
}

static char verifyData(int checksum);
static void interpretCommand();

static char RX_StatusFlags;
static int dataPointer;
static volatile int checksum;

static char senddata;

int protocol_recv(unsigned char dataByte)
{
  if (dataPointer == 0)                                // first byte is the size of the Core
                                                           // packet
      {
          if (dataByte != 0x80)                            // first byte in packet should be 0x80
          {
              senddata = HEADER_INCORRECT;
              RX_StatusFlags = RX_ERROR_RECOVERABLE;
          }
          else
          {
              dataPointer++;
          }
      }
      else if (dataPointer == 1)                           // first byte is the size of the Core
                                                           // packet
      {
          BSL430_BufferSize = dataByte;
          dataPointer++;
      }
      else if (dataPointer == 2)
      {
          BSL430_BufferSize |= (int)dataByte << 8;
          if (BSL430_BufferSize == 0)
          {
              senddata = PACKET_SIZE_ZERO;
              RX_StatusFlags = RX_ERROR_RECOVERABLE;
          }
          if (BSL430_BufferSize > MAX_BUFFER_SIZE)         // For future devices that might need
                                                           // smaller packets
          {
              senddata = PACKET_SIZE_TOO_BIG;
              RX_StatusFlags = RX_ERROR_RECOVERABLE;
          }
          dataPointer++;
      }
      else if (dataPointer == (BSL430_BufferSize + 3))
      {
          // if the pointer is pointing to the Checksum low data byte which resides
          // after 0x80, rSize, Core Command.
          checksum = dataByte;
          dataPointer++;
      }
      else if (dataPointer == (BSL430_BufferSize + 4))
      {
          // if the pointer is pointing to the Checksum low data byte which resides
          // after 0x80, rSize, Core Command, CKL.
          checksum = checksum | dataByte << 8;
          if (verifyData(checksum))
          {
              if ((RAM_Buf[0] & 0xF0) == PI_COMMAND_UPPER)
              {
                  RX_StatusFlags = RX_PACKET_ONGOING;
                  dataByte = 0;
                  dataPointer = 0;
                  checksum = 0;
              }
              else
              {
                  senddata = ACK;
                  RX_StatusFlags = DATA_RECEIVED;
              }
          }
          else
          {
              senddata = CHECKSUM_INCORRECT;
              RX_StatusFlags = RX_ERROR_RECOVERABLE;
          }
      }
      else
      {
          RAM_Buf[dataPointer - 3] = dataByte;
          dataPointer++;
      }


  if (RX_StatusFlags == RX_PACKET_ONGOING)
    return 0;
  else
    return 1;
}

PROCESS_THREAD(protocol_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
    RX_StatusFlags = RX_PACKET_ONGOING;
    dataPointer = 0;
    checksum = 0;

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        
    sendByte(senddata);
    if (RX_StatusFlags & DATA_RECEIVED)
    {
      interpretCommand();
    }
  }

  PROCESS_END();
}


/*******************************************************************************
* *Function:    sendMessage
* *Description: Sends a Reply message with attached information
* *Parameters:
*           char message    the message to send
*******************************************************************************/

static void sendMessage(char message)
{
    BSL430_SendBuffer[0] = BSL_MESSAGE_REPLY;
    BSL430_SendBuffer[1] = message;
    PI_sendData(2);
}

/*******************************************************************************
* *Function:    sendData
* *Description: Sends the data in the data buffer
* *Parameters:
*           int size    the number of bytes in the buffer
*******************************************************************************/

static void PI_sendData(int size)
{
    int i;

    sendByte(0x80);
    sendByte(size & 0xFF);
    sendByte(size >> 8 & 0xFF);
    CRCINIRES = 0xFFFF;
    for (i = 0; i < size; i++){
        CRCDIRB_L = RAM_Buf[i];
        sendByte(RAM_Buf[i]);
    }
    i = CRCINIRES;
    sendByte(i & 0xFF);
    sendByte(i >> 8 & 0xFF);
}


/*******************************************************************************
* *Function:    verifyData
* *Description: verifies the data in the data buffer against a checksum
* *Parameters:
*           int checksum    the checksum to check against
****************Returns:
*           1 checksum parameter is correct for data in the data buffer
*           0 checksum parameter is not correct for the data in the buffer
*******************************************************************************/

static char verifyData(int checksum)
{
    int i;

    CRCINIRES = 0xFFFF;
    for (i = 0; i < BSL430_BufferSize; i++)
    {
        CRCDIRB_L = RAM_Buf[i];
    }
    return (CRCINIRES == checksum);
}


static int PI_getBufferSize()
{
    return MAX_BUFFER_SIZE;
}

/*******************************************************************************
* *Function:    sendDataBlock
* *Description: Fills the SendBuffer array with bytes from the given parameters
*             Sends the data by calling the PI, or sends an error
****************Parameters:
*           unsigned long addr    The address from which to begin reading the block
*           int length            The number of bytes to read
*******************************************************************************/

static void sendDataBlock(const char* addr, unsigned int length)
{
    const char* endAddr = addr + length;
    unsigned int bytes;

    while (addr < endAddr)
    {
        if ((endAddr - addr) > PI_getBufferSize() - 1)
        {
            bytes = PI_getBufferSize() - 1;
        }
        else
        {
            bytes = (endAddr - addr);
        }

        for(int i = 0; i < bytes; i++)
            BSL430_SendBuffer[i + 1] = addr[i];
        BSL430_SendBuffer[0] = BSL_DATA_REPLY;
        PI_sendData(bytes + 1);
        addr += bytes;
    }
}

static const char PROTOCOL_Version[4] = { 0x01, 0x00, 0x00, 0x02 };

static int fd_handle;

static void file_begin(char *name, int mode)
{
  name[BSL430_BufferSize - 4] = 0;

    if (mode == 1)
        fd_handle = cfs_open(name, CFS_WRITE | CFS_APPEND);
    else
        fd_handle = cfs_open(name, CFS_READ);

    if (fd_handle == -1)
        sendMessage(UNKNOWN_ERROR);
    else
    {
        sendMessage(ACK);
    }
}

static void file_write(unsigned long addr, char* data, char fastWrite)
{
    char returnValue;
    if (fd_handle == -1)
    {
        returnValue = UNKNOWN_ERROR;
    }
    else
    {
        unsigned int length = BSL430_BufferSize - 4;
        cfs_seek(fd_handle, addr, CFS_SEEK_SET);

        if (cfs_write(fd_handle, data, length) == length)
            returnValue = ACK;
        else
            returnValue = UNKNOWN_ERROR;
    }

    if (!fastWrite)
    {
        sendMessage(returnValue);
    }
}

static void file_close()
{
    if (fd_handle == -1)
        sendMessage(UNKNOWN_ERROR);
    else
    {
        cfs_close(fd_handle);
        sendMessage(ACK);
    }
}

static void file_read(unsigned long addr, unsigned int length)
{
    if (fd_handle == -1)
    {
        sendMessage(UNKNOWN_ERROR);
    }
    else
    {
        unsigned long endAddr = addr + length;
        unsigned int bytes;

        while (addr < endAddr)
        {
            if ((endAddr - addr) > PI_getBufferSize() - 1)
            {
                bytes = PI_getBufferSize() - 1;
            }
            else
            {
                bytes = (endAddr - addr);
            }

            cfs_seek(fd_handle, addr, CFS_SEEK_SET);
            int length = cfs_read(fd_handle, &BSL430_SendBuffer[1], bytes);
            bytes = length;

            BSL430_SendBuffer[0] = BSL_DATA_REPLY;
            PI_sendData(bytes + 1);
            addr += bytes;
        }
    }
}

static void file_remove(char * name)
{
    name[BSL430_BufferSize - 4] = 0;

    if (cfs_remove(name) == -1)
        sendMessage(UNKNOWN_ERROR);
    else
        sendMessage(ACK);
}

static void interpretCommand()
{

    /*
    REQUEST
    0 - CMD
    1 - AL
    2 - AM
    3 - AH
    4 - LL
    5 - LH
    6 -- BSL_BufferSize - DATA
    */
    unsigned char command = BSL430_ReceiveBuffer[0];
    unsigned long addr = BSL430_ReceiveBuffer[1];

    addr |= ((unsigned long)BSL430_ReceiveBuffer[2]) << 8;
    addr |= ((unsigned long)BSL430_ReceiveBuffer[3]) << 16;

    /*----------------------------------------------------------------------------*/
    switch (command)
    {
        case TX_BSL_VERSION:              // Transmit BSL Version array
            sendDataBlock(PROTOCOL_Version, 4);
            break;
        case TX_FILE_BEGIN:
            {            
            file_begin(&BSL430_ReceiveBuffer[4], addr);
            break;
            }
        case RX_DATA_BLOCK:
            {
            file_write(addr, &BSL430_ReceiveBuffer[4], 0);
            break;
            }
        case RX_DATA_BLOCK_FAST:
            {
            file_write(addr, &BSL430_ReceiveBuffer[4], 1);
            break;
            }
        case TX_FILE_END:
            {
            file_close();
            break;
            }
        case TX_FILE_REMOVE:
            {
            file_remove(&BSL430_ReceiveBuffer[4]);
            break;
            }
        case TX_DATA_BLOCK:
            {
            unsigned int length;
            length = BSL430_ReceiveBuffer[4];
            length |= BSL430_ReceiveBuffer[5] << 8;
            file_read(addr, length);
            break;
            }
        case TX_LOG_GET:
            sendDataBlock(Log_Buf, LogWrite);
            LogWrite = 0;
            break;
        case MASS_ERASE:
            {
                cfs_coffee_format();
                sendMessage(SUCCESSFUL_OPERATION);
                break;
            }
        default:
            sendMessage(UNKNOWN_COMMAND);
    }
}

#if 0
static void interpretPI_Command()
{
    char command = RAM_Buf[0];

    if (command == CHANGE_BAUD_RATE)
    {
        char rate = RAM_Buf[1];
        if (rate > BAUD_4800 && rate <= BAUD_115200)
        {
            sendMessage(ACK);
            uart_changerate(rate);
        }
        else
            sendMessage(UNKNOWN_BAUD_RATE);
    }
}
#endif