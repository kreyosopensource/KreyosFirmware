#include "BSL_UART_Protocol_5xx.h"
#include "BSL_USB_Protocol_5xx.h"
#include "BSL_UART_Protocol_ROM.h"
#include "BSL_IO_UART.h"
#include "BSL_IO_USB.h"
//#include "BSL_5xx_Command_Definitions.h"
#include "BSL_Commands_5xx.h"
#include "BSL_Commands_ROM.h"
//#include "BSL_Function_pointers.h"
#include "BSL_Interface.h"
#include "TextFileIO.h"
#include "BSL_Definitions.h"
#include <string.h>

int BSL_Version;

void (*BSL_set_TX_Packet)( unsigned char (*txFunc)( dataBuffer db ) );
void (*BSL_set_RX_Packet)( unsigned char (*rxFunc)( dataBuffer *db ) );

unsigned int BSL_bufferSize;
unsigned char versionString[50];

unsigned char* BSL_TX_BSL_Version_String()
{
  dataBuffer rx_buf;
  if( BSL_TX_BSL_Version() == BSL_ACK )
  {
	rx_buf = BSL_get_RX_Buffer();
    if((BSL_Version == FAMILY_5438)||(BSL_Version == FAMILY_FLASH))
	{
	  strcpy(versionString, "Vendor:[xx],CI:[xx],API:[xx],PI:[xx]");
      if( rx_buf.data[1] == 0x00 )
	  {
        versionString[8] = 'T';
        versionString[9] = 'I';
	  }
	  else
	  {
	    // Vend
	    versionString[8] = ((rx_buf.data[1]>>4)&0x0F)+'0';
	    versionString[9] = ((rx_buf.data[1])&0x0F)+'0';
	  }
	  // CI
	  versionString[16] = ((rx_buf.data[2]>>4)&0x0F)+'0';
	  versionString[17] = ((rx_buf.data[2])&0x0F)+'0';
	  // API
	  versionString[25] = ((rx_buf.data[3]>>4)&0x0F)+'0';
	  versionString[26] = ((rx_buf.data[3])&0x0F)+'0';
	  // PI
	  versionString[33] = ((rx_buf.data[4]>>4)&0x0F)+'0';
	  versionString[34] = ((rx_buf.data[4])&0x0F)+'0';
	}
	else if (BSL_Version == FAMILY_ROM)
	{
	}

  }
  else
  {
    strcpy( versionString, "ERROR" );
  }
  return versionString;
}

signed int BSL_TX_BufferSize()
{
  if((BSL_Version == FAMILY_5438)||(BSL_Version == FAMILY_FLASH))
  {
    if ( Flash_TX_BufferSize() == BSL_ACK )
	{
      dataBuffer rx_buf;
	  signed int retValue;
	  rx_buf = BSL_get_RX_Buffer();
      retValue = rx_buf.data[1];
      retValue |= rx_buf.data[2]<<8;
	  return retValue;
	}
	else
	{
      return -2;
	}
  }
  else
  {
    return -1;
  } // wrong family
}

// Read from BSL to text file
unsigned int BSL_TX_TXT_File( char* fileName, unsigned int addr, unsigned int length )
{
  unsigned char buf[(1024*512)];
  DWORD start, stop;
  float seconds, kbs;

  // below is the better way to do things, but crashes currently
  //unsigned char *buf = malloc( (size_t)length);

  
  openTI_TextForWrite( fileName );
  /*
  for( curAddr = addr; curAddr < (addr+length); curAddr+=curLength)
  {
	if( (addr+length)-curAddr < BSL_bufferSize )
	{
      curLength = (addr+length)-curAddr;
	}
	*/
  start = GetTickCount();
  if ( BSL_SetVerbose == &USB_setVerbose )
  {
    if ( BSL_TX_DataBlock ( addr, length, buf ) == BSL_ACK )
  	{
  	  writeTI_TextFile( addr, buf, length); 
  	}
  	else
  	{
  	  return 0xFF;
  	}
  } // if USB
  else
  {
    unsigned int curLength = BSL_bufferSize;
	  unsigned int curAddr;
    for( curAddr = addr; curAddr < (addr+length); curAddr+=curLength)
	  {
  	  if( (addr+length)-curAddr < BSL_bufferSize )
  	  {
          curLength = (addr+length)-curAddr;
  	  }
  	  if ( BSL_TX_DataBlock ( curAddr, curLength, &buf[curAddr-addr] ) == BSL_ACK )
  	  {
  	  }
  	  else
  	  {
  	    return 0xFF;
  	  }
	    //writeTI_TextFile( curAddr, buf, curLength); 
	  }// for
	  writeTI_TextFile( addr, buf, length); 

  }
  //} // for whole file

  stop = GetTickCount();
  seconds = ((float)stop-(float)start)/(float)1000;
  kbs = ((float)length/(float)1024)/(float)seconds;
  printf( "Read %i bytes in %.2f seconds [%.2f Kbytes/s]\n", length, seconds, kbs);
  endTI_TextWrite();
  return BSL_ACK;
}

int ext_match(const char *name, const char *ext)
{
  size_t nl = strlen(name), el = strlen(ext);
  return nl >= el && !strcmp(name + nl - el, ext);
}

unsigned int BSL_RX_TXT_File( char* fileName, unsigned char fast)
{
  DWORD start, stop;
  float seconds, kbs;
  DataBlock data;
  unsigned int numBytes=0;
  //int i;
  //printf( "\n");
  if (ext_match(fileName, "txt"))
  openTI_TextForRead( fileName );
  else if (ext_match(fileName, "bin"))
    open_BinaryForRead(fileName);
  else
  {
    printf("file format is unknown.\n");
    return -1;
  }
  start = GetTickCount();
  while( moreDataToRead() )
  {
    data = readTI_TextFile(BSL_bufferSize);
  	numBytes += data.numberOfBytes;
  	/*
  	for( i = 0; i < data.numberOfBytes; i++)
  	{
      printf( "%2.2x ", data.data[i] );
  	}
  	*/
  	if( !fast)
  	{
      int ret;
      for(int retry = 0; retry < 10; retry++)
      {
        ret = BSL_RX_DataBlock( data );
        if (ret == BSL_ACK)
          break;
        printf("retry write @%x\n", data.startAddr);
      }

  	  if (  ret != BSL_ACK )
  	  {
          return data.startAddr;
  	  }
  	}
  	else
  	{
        BSL_RX_DataBlock_Fast( data );
  	}
    stop = GetTickCount();
    seconds = ((float)stop-(float)start)/(float)1000;
    kbs = ((float)numBytes/(float)1024)/(float)seconds;
  }

  printf( "Wrote %i bytes in %.2f seconds [%.2f Kbytes/s]\n", numBytes, seconds, kbs);

  return BSL_ACK;
}


void BSL_setFamily( unsigned int family )
{
  if( family == FAMILY_ROM )
  {
    BSL_RX_Password = &ROM_RX_Password;
	BSL_RX_DataBlock = &ROM_RX_DataBlock;
    BSL_TX_DataBlock = &ROM_TX_DataBlock;
    BSL_eraseSegment = &ROM_eraseSegment;
    BSL_massErase = &ROM_massErase;
	BSL_eraseCheck = &ROM_eraseCheck;
	BSL_setMemOffset = &ROM_setMemOffset;

	BSL_set_TX_Packet = &ROM_set_TX_Packet;
	BSL_set_RX_Packet = &ROM_set_RX_Packet;
	BSL_get_RX_Buffer = &ROM_get_RX_Buffer;
	BSL_eraseMainOrInfo = &ROM_eraseMainOrInfo;
	BSL_Version = FAMILY_ROM;
	BSL_bufferSize = 240;
  }
  else if ((family == FAMILY_FLASH)||(family == FAMILY_5438))
  {
	BSL_set_TX_Packet = &Flash_set_TX_Packet;
	BSL_set_RX_Packet = &Flash_set_RX_Packet;
    BSL_TX_BSL_Version = &Flash_TX_BSL_Version;
    BSL_TX_DataBlock = &Flash_TX_DataBlock;
    BSL_LoadPC = &Flash_LoadPC;
    BSL_CRC_Check = &Flash_CRC_Check;
    BSL_massErase = &Flash_massErase;
    BSL_toggleInfo = &Flash_toggleInfo;
    BSL_eraseSegment = &Flash_eraseSegment;
    BSL_RX_Password = &Flash_RX_Password;
    BSL_RX_DataBlock = &Flash_RX_DataBlock;
    BSL_RX_DataBlock_Fast = &Flash_RX_DataBlock_Fast;
	BSL_get_RX_Buffer = &Flash_get_RX_Buffer;
    BSL_changeBaudRate = &Flash_changeBaudRate;
	BSL_bufferSize = 240;
	BSL_Version = FAMILY_FLASH;
  }
  if ( family == FAMILY_5438 )
  {
    BSL_RX_Password = &Flash_RX_Password_5438;
    //BSL_TX_BufferSize = &Flash_TX_BufferSize;
	BSL_Version = FAMILY_5438;
  }
}

void BSL_setCom( unsigned int com )
{
  if( com == COM_UART )
  {
	BSL_SetVerbose = &UART_setVerbose;
	BSL_initialize_BSL = &UART_initialize_BSL;
  BSL_Reset = &UART_reset;

	if (BSL_Version == FAMILY_ROM )
	{
      BSL_set_TX_Packet( &UART_ROM_TXPacket);
      BSL_set_RX_Packet( &UART_ROM_RXPacket);
	}
	else if( (BSL_Version == FAMILY_5438)||(BSL_Version == FAMILY_FLASH) )
	{
      BSL_set_TX_Packet( &UART_5xx_TXPacket);
      BSL_set_RX_Packet( &UART_5xx_RXPacket);
	  if( BSL_Version == FAMILY_5438)
	  {
	    BSL_initialize_BSL = &UART_initialize_BSL_5438;
	  }
	}
  }
  else if ( com == COM_USB )
  {
	BSL_SetVerbose = &USB_setVerbose;
	BSL_initialize_BSL = &USB_initialize_BSL;
    BSL_set_TX_Packet( &USB_5xx_TXPacket);
    BSL_set_RX_Packet( &USB_5xx_RXPacket);
	BSL_bufferSize = 48;

  }
}


// Read from BSL to text file
unsigned int MY_TX_TXT_File( char* fileName, char *dest)
{
  unsigned char buf[(1024*512)];
  DWORD start, stop;
  float seconds, kbs;
  FILE *fp = fopen(dest, "wb");
  // below is the better way to do things, but crashes currently
  //unsigned char *buf = malloc( (size_t)length);

  start = GetTickCount();
  unsigned int curLength = BSL_bufferSize;
  unsigned int curAddr = 0;
  MY_OpenFile(fileName, 0); // read
  while(1)
  {
    if ( BSL_TX_DataBlock ( curAddr, curLength, &buf[curAddr] ) != BSL_ACK )
    {
      break;
    }
    curAddr+=curLength;
  }// for

  MY_CloseFile();

  stop = GetTickCount();
  seconds = ((float)stop-(float)start)/(float)1000;
  kbs = ((float)curAddr/(float)1024)/(float)seconds;
  printf( "Read %i bytes in %.2f seconds [%.2f Kbytes/s]\n", curAddr, seconds, kbs);
  return BSL_ACK;
}

unsigned int MY_RX_TXT_File( char* fileName, char* srcfile, unsigned char fast)
{
  unsigned int ret;
  DWORD start, stop;
  float seconds, kbs;
  DataBlock data;
  unsigned int numBytes=0;

  ret = open_BinaryForRead( srcfile );
  if (ret != OPERATION_SUCCESSFUL)
  {
    printf("open src failed\n");
    return ret;
  }
  ret = MY_OpenFile(fileName, 1); // write
  if (ret != BSL_ACK)
  {
    printf("open flash failed\n");
    return ret;
  }
  start = GetTickCount();
  while( moreDataToRead() )
  {
    data = readTI_TextFile(BSL_bufferSize);
    numBytes += data.numberOfBytes;
    if( !fast)
    {
      int ret;
      for(int retry = 0; retry < 3; retry++)
      {
        ret = BSL_RX_DataBlock( data );
        if (ret == BSL_ACK)
          break;
        printf("retry write @%x\n", data.startAddr);
      }

      if ( ret != BSL_ACK )
      {
          return data.startAddr;
      }
    }
    else
    {
        BSL_RX_DataBlock_Fast( data );
    }  
  }
  MY_CloseFile();
  stop = GetTickCount();
  seconds = ((float)stop-(float)start)/(float)1000;
  kbs = ((float)numBytes/(float)1024)/(float)seconds;
  printf( "Wrote %i bytes in %.2f seconds [%.2f Kbytes/s]\n", numBytes, seconds, kbs);
  return BSL_ACK;
}