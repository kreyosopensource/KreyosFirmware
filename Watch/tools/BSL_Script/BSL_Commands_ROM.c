#include "BSL_Commands_ROM.h"
#include "BSL_Command_Definitions_ROM.h"


#define DUMMY_DATA 0x00

dataBuffer TXBuffer;
dataBuffer RXBuffer;
unsigned int answer;

unsigned char (*BSL_RX_Packet)( dataBuffer *db );
unsigned char (*BSL_TX_Packet)( dataBuffer  db );

unsigned char ROM_setMemOffset( unsigned long int addr)
{
  TXBuffer.data[0] = SET_MEM_OFFSET;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = DUMMY_DATA;
  TXBuffer.data[4] = DUMMY_DATA;
  TXBuffer.data[5] = (unsigned char)(addr&0xFF);
  TXBuffer.data[6] = (unsigned char)((addr>>8)&0xFF);
  TXBuffer.size = 7;
  answer = BSL_TX_Packet( TXBuffer );
  if( answer == ROM_ACK )
  {
    answer = BSL_ACK;
  }
  return answer; 
}

unsigned char ROM_eraseCheck( unsigned long int addr, unsigned long int length )
{
  TXBuffer.data[0] = ERASE_CHECK;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = (unsigned char)(addr&0xFF);
  TXBuffer.data[4] = (unsigned char)((addr>>8)&0xFF);
  TXBuffer.data[5] = (unsigned char)(length&0xFF);
  TXBuffer.data[6] = (unsigned char)((length>>8)&0xFF);
  TXBuffer.size = 7;
  answer = BSL_TX_Packet( TXBuffer );
  if( answer == ROM_ACK )
  {
    answer = BSL_ACK;
  }
  return answer; 

}

unsigned char ROM_massErase()
{
  unsigned char answer;
  TXBuffer.data[0] = MASS_ERASE;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = 0x00;
  TXBuffer.data[4] = 0xFF;
  TXBuffer.data[5] = 0x06;
  TXBuffer.data[6] = 0xA5;
  TXBuffer.size = 7;
  answer = BSL_TX_Packet( TXBuffer );
  if( answer == ROM_ACK )
  {
    answer = BSL_ACK;
  }
  return answer; 
}

unsigned char ROM_eraseMainOrInfo( unsigned long int addr )
{
  unsigned char answer;
  TXBuffer.data[0] = ERASE_MAIN_OR_INFO;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = (unsigned char)(addr&0xFF);
  TXBuffer.data[4] = (unsigned char)((addr>>8)&0xFF);
  TXBuffer.data[5] = 0x04;
  TXBuffer.data[6] = 0xA5;
  TXBuffer.size = 7;
  answer = BSL_TX_Packet( TXBuffer );
  if( answer == ROM_ACK )
  {
    answer = BSL_ACK;
  }
  return answer;
}

unsigned char ROM_eraseSegment( unsigned long int addr )
{  
  unsigned char answer;
  TXBuffer.data[0] = ERASE_SEGMENT;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = (unsigned char)(addr&0xFF);
  TXBuffer.data[4] = (unsigned char)((addr>>8)&0xFF);
  TXBuffer.data[5] = 0x02;
  TXBuffer.data[6] = 0xA5;
  TXBuffer.size = 7;
  answer = BSL_TX_Packet( TXBuffer );
  if( answer == ROM_ACK )
  {
    answer = BSL_ACK;
  }
  return answer;
}


unsigned char ROM_TX_DataBlock( unsigned long int addr, unsigned long int length )
{
  unsigned char answer;
  TXBuffer.data[0] = TX_DATA_BLOCK;
  TXBuffer.data[1] = 0x04;
  TXBuffer.data[2] = 0x04;
  TXBuffer.data[3] = (unsigned char)(addr&0xFF);
  TXBuffer.data[4] = (unsigned char)((addr>>8)&0xFF);
  TXBuffer.data[5] = (unsigned char)length;
  TXBuffer.data[6] = 0;
  TXBuffer.size = 7;
  answer = BSL_TX_Packet(TXBuffer);
  if( answer != 0x80 )   // 0x80 means it is starting to send reply data
  {
    return answer;
  }
  else
  {
    answer = BSL_RX_Packet(&RXBuffer);
	if( answer != ROM_ACK )
	{
      return answer;
	}
	else
	{
      return BSL_ACK;
	}
  }

}
/*******************************************************************************
*******************************************************************************/
unsigned char ROM_RX_DataBlock(DataBlock data)
{
  unsigned int i;
  unsigned char retValue;
  TXBuffer.data[0] = RX_DATA_BLOCK;
  TXBuffer.data[1] = (data.numberOfBytes+4) & 0xFF;
  TXBuffer.data[2] = (data.numberOfBytes+4) & 0xFF;
  TXBuffer.data[3] = (unsigned char)(data.startAddr & 0xFF);
  TXBuffer.data[4] = (unsigned char)((data.startAddr>>8) & 0xFF);
  TXBuffer.data[5] = (data.numberOfBytes) & 0xFF;
  TXBuffer.data[6] = 0;
  for( i = 0; i < data.numberOfBytes; i++ )
  {
    TXBuffer.data[i+7] = data.data[i];
  }
  TXBuffer.size = data.numberOfBytes+7;
  retValue = BSL_TX_Packet( TXBuffer );
  if( retValue == ROM_ACK )
  {
    retValue = BSL_ACK;
  }
  return retValue;

}

/*******************************************************************************
*******************************************************************************/
unsigned char ROM_RX_Password( DataBlock data )
{
  unsigned int i;
  unsigned char retValue;
  TXBuffer.data[0] = RX_PASSWORD;
  TXBuffer.data[1] = 0x24;
  TXBuffer.data[2] = 0x24;
  TXBuffer.data[3] = DUMMY_DATA;
  TXBuffer.data[4] = DUMMY_DATA;
  TXBuffer.data[5] = DUMMY_DATA;
  TXBuffer.data[6] = DUMMY_DATA;
  for( i = 0; i < data.numberOfBytes; i++ )
  {
    TXBuffer.data[i+7]=data.data[i];
  }
  TXBuffer.size = data.numberOfBytes+7;
  retValue = BSL_TX_Packet( TXBuffer );
  if( retValue == ROM_ACK )
  {
    retValue = BSL_ACK;
  }
  return retValue;
}

/*******************************************************************************
*******************************************************************************/
dataBuffer ROM_get_RX_Buffer()
{
  return RXBuffer;
}
/*******************************************************************************
*******************************************************************************/
void ROM_set_TX_Packet( unsigned char (*txFunc)( dataBuffer  db ) )
{
  BSL_TX_Packet = txFunc;
}

/*******************************************************************************
*******************************************************************************/
void ROM_set_RX_Packet( unsigned char (*rxFunc)( dataBuffer  *db ) )
{
  BSL_RX_Packet = rxFunc;
}