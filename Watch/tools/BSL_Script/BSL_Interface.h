#include "BSL_Definitions.h"

#define FAMILY_5438  1
#define FAMILY_ROM   2
#define FAMILY_FLASH 3

#define COM_UART 1
#define COM_USB  2

#define TEST_PIN 1
#define RST_PIN  2

void BSL_setFamily( unsigned int family );
void BSL_setCom( unsigned int com );

unsigned int BSL_TX_TXT_File( char* fileName, unsigned int addr, unsigned int length );
unsigned int BSL_RX_TXT_File( char* fileName, unsigned char fast);
signed int BSL_TX_BufferSize();
unsigned char* BSL_TX_BSL_Version_String();

dataBuffer (*BSL_get_RX_Buffer)();
int (*BSL_initialize_BSL)( unsigned char* com);
void (*BSL_SetVerbose)( unsigned int verb ); 
void (*BSL_Revert)(int pin);
void (*BSL_Reset)();

//unsigned char (*BSL_TX_BufferSize)();
unsigned char (*BSL_TX_BSL_Version)();
unsigned char (*BSL_TX_DataBlock)( unsigned long int addr, unsigned long int length, unsigned char *buffer );
unsigned char (*BSL_eraseCheck)( unsigned long int addr, unsigned long int length );
unsigned char (*BSL_LoadPC)( unsigned long int addr );
unsigned char (*BSL_setMemOffset)( unsigned long int addr );
unsigned char (*BSL_CRC_Check)( unsigned long int addr, unsigned long int length );
unsigned char (*BSL_massErase)();
unsigned char (*BSL_toggleInfo)();
unsigned char (*BSL_eraseSegment)( unsigned long int addr );
unsigned char (*BSL_eraseMainOrInfo)( unsigned long int addr );
unsigned char (*BSL_RX_Password)( DataBlock data );
unsigned char (*BSL_RX_DataBlock)(DataBlock data);
unsigned char (*BSL_RX_DataBlock_Fast)(DataBlock data);
unsigned char (*BSL_changeBaudRate)( unsigned int rate );

static DataBlock default_pass = {								
	{(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,
	(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,
	(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,
	(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF,(char)0xFF},
    32,
	0xFFE0
};