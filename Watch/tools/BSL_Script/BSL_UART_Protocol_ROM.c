#include "BSL_UART_Protocol_ROM.h"
#include "BSL_Command_Definitions_ROM.h"
#include "BSL_IO_UART.h"
#include "CRC.h"

unsigned char UART_ROM_RXPacket( dataBuffer *db )
{
  unsigned char scrap_var;
  unsigned int i;
  if( UART_GetVerbose() == 1 )
  {
    printf("\n-------------------------------------------------\n");
  }
  // 0x80 is read by TXPacket below
  scrap_var = readByte();             // first byte is always dummy
  scrap_var = readByte();             // second is size, disposable
  db->size = readByte();              // next size byte
  for( i = 0; i < db->size; i++ )
  {
    db->data[i] = readByte();
  }
  scrap_var = readByte();             // CKL to do: check checksum
  scrap_var = readByte();             // CKH
  if( UART_GetVerbose() == 1 )
  {
    printf("\n-------------------------------------------------\n");
  }
  return ROM_ACK;
}


unsigned char UART_ROM_TXPacket( dataBuffer db )
{  
  unsigned char answer;
  unsigned int i;
  if( UART_GetVerbose() == 1 )
  {
    printf("\n-------------------------------------------------\n");
  }
  ROM_crc_init();
  writeByte( (unsigned char)0x80 );
  answer = readByte();
  if( answer != ROM_ACK )
  {
    return answer;
  } 
  writeByte( (unsigned char)0x80 );
  ROM_crcByte( 0x80 );
  for( i = 0; i < db.size; i++ )
  {
	ROM_crcByte( db.data[i] );
    writeByte( db.data[i] );
  } // for
  writeByte( ROM_getLowByte() );
  writeByte( ROM_getHighByte() );
  if( UART_GetVerbose() == 1 )
  {
    printf("\n-------------------------------------------------\n");
  }
  return readByte();
}