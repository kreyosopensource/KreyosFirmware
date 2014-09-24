#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "BSL_Definitions.h"

#define OPERATION_SUCCESSFUL 0
#define TXT_EOF -1
#define ERROR_OPENING_FILE -2
#define NO_DATA_READ -3

int openTI_TextForRead( char *filename );
int openTI_TextForWrite( char *filename );
void endTI_TextWrite();
void closeTI_Text();
int moreDataToRead();
void writeTI_TextFile( int addr, unsigned char *data, int length );
DataBlock readTI_TextFile(int bytesToRead);

// new binary file operations for flash
int open_BinaryForRead( char *filename );