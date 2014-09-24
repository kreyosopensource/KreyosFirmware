#include <stdint.h>
#include <string.h>
#include <stdio.h>


#define OPERATION_SUCCESSFUL 0
#define TXT_EOF -1
#define ERROR_OPENING_FILE -2
#define NO_DATA_READ -3

struct datablock
{
  uint32_t currentAddr;
  uint32_t size;
  unsigned int offset;
  unsigned char *data;
};

extern struct datablock blocks[20];

int openTI_TextForRead( char *filename );
int openTI_TextForWrite( char *filename );
void endTI_TextWrite();
void closeTI_Text();
int moreDataToRead();
void writeTI_TextFile( int addr, unsigned char *data, int length );

// new binary file operations for flash
int open_BinaryForRead( char *filename );
