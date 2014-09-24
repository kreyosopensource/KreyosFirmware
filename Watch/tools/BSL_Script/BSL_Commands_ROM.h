#include "BSL_Definitions.h"

unsigned char ROM_RX_Password( DataBlock data );
unsigned char ROM_RX_DataBlock(DataBlock data);
unsigned char ROM_TX_DataBlock( unsigned long int addr, unsigned long int length );
unsigned char ROM_eraseSegment( unsigned long int addr );
unsigned char ROM_eraseMainOrInfo( unsigned long int addr );
unsigned char ROM_setMemOffset( unsigned long int addr );
unsigned char ROM_massErase();
unsigned char ROM_eraseCheck( unsigned long int addr, unsigned long int length );

dataBuffer ROM_get_RX_Buffer();

void ROM_set_TX_Packet( unsigned char (*txFunc)( dataBuffer  db ) );
void ROM_set_RX_Packet( unsigned char (*rxFunc)( dataBuffer  *db ) );

