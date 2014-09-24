#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define BSL_ACK 0x00
#define MAX_BUFFER_SIZE 255

typedef struct {
  unsigned long int size;
  unsigned char data [50 + MAX_BUFFER_SIZE];
} dataBuffer;

typedef struct DataBlock_ {
  unsigned char data[256];
  unsigned int numberOfBytes;
  unsigned long int startAddr;

} DataBlock;

#endif