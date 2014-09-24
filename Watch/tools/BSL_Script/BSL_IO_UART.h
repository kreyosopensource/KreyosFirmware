#include <string.h>
#include <stdio.h>

int UART_initialize_BSL_5438(unsigned char* comPort);
int UART_initialize_BSL(unsigned char* comPort);
unsigned char readByte_s(int timeout);
unsigned char readByte();
void writeBytes( unsigned char *byte, int size );
void writeByte( unsigned char byte );
void changeCommBaudRate( unsigned int rate );
void invokeBSL();
void UART_setVerbose( unsigned int val );
unsigned int UART_GetVerbose();
void clearBuffers(void);
void ReadGargbageUntilTimeout(void);
void UART_revert(unsigned int pin);
void UART_reset();

/* Size of internal WINDOWS-Comm-Buffer: */
#define QUEUE_SIZE       512 
#define MAX_UART_FRAME_SIZE   QUEUE_SIZE
#define TIMEOUT_ERROR    0xEE

//unsigned char dataBuffer[MAX_FRAME_SIZE];


//#define DEBUG_VERBOSE
//#define DEBUG_ERRORS
