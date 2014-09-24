#include "BSL_Definitions.h"

dataBuffer Flash_get_RX_Buffer();
void Flash_set_TX_Packet( unsigned char (*txFunc)( dataBuffer  db ) );
void Flash_set_RX_Packet( unsigned char (*rxFunc)( dataBuffer  *db ) );

unsigned char Flash_TX_BufferSize();
unsigned char Flash_TX_BSL_Version();
unsigned char Flash_TX_DataBlock( unsigned long int addr, unsigned long int length, unsigned char *buffer );
unsigned char Flash_LoadPC( unsigned long int addr );
unsigned char Flash_CRC_Check( unsigned long int addr, unsigned long int length );
unsigned char Flash_massErase();
unsigned char Flash_toggleInfo();
unsigned char Flash_eraseSegment( unsigned long int addr );
unsigned char Flash_RX_Password_5438( DataBlock data );
unsigned char Flash_RX_Password( DataBlock data );
unsigned char Flash_RX_DataBlock(DataBlock data);
unsigned char Flash_RX_DataBlock_Fast(DataBlock data);
unsigned char Flash_changeBaudRate( unsigned int rate );

unsigned char Send_Unknown(void);
