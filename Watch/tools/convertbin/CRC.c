/*==========================================================================*\
|                                                                            |
|                                                                            |
| PC-side Bootstrap Loader communication Application                         |
|                                                                            |
| See main.c for full version and legal information                          |
|                                                                            |
\*==========================================================================*/

#include "CRC.h"

unsigned short int crc;

unsigned int oddByte = 0;
unsigned char highByte = 0;
unsigned char lowByte = 0;
/*******************************************************************************
*Function:    crcInit
*Description: initiailzes the CRC
*Parameters: 
*             unsigned short int init  The CRC init value
*Returns:
*             none
*******************************************************************************/
void Flash_crcInit(unsigned short int init )
{
  crc = init;
}

/*******************************************************************************
*Function:    crcInput
*Description: inputs one byte to the ongoing CRC
*Parameters: 
*             char data             A byte to add to the ongoing CRC
*Returns:
*             none
*******************************************************************************/
void Flash_crcInput(unsigned char data)
{
  unsigned short int x;
  x = ((crc>>8) ^ data) & 0xff;
  x ^= x>>4;
  crc = (crc << 8) ^ (x << 12) ^ (x <<5) ^ x;
}

/*******************************************************************************
*Function:    getHighByte
*Description: returns the high byte from the 16 bit CRC
*Parameters: 
*             none
*Returns:
*             the high byte of the 16 bit CRC result
*******************************************************************************/
unsigned char Flash_getHighByte()
{
  return (unsigned char)((crc>>8)&0xFF);
}

/*******************************************************************************
*Function:    getLowByte
*Description: returns the low byte from the 16 bit CRC
*Parameters: 
*             none
*Returns:
*             the low byte of the 16 bit CRC result
*******************************************************************************/
unsigned char Flash_getLowByte()
{
  return (unsigned char)(crc&0xFF);
}


/*******************************************************************************
*
*******************************************************************************/
void ROM_crc_init()
{
  oddByte = 0;
  highByte = 0;
  lowByte = 0;
}

/*******************************************************************************
*
*******************************************************************************/
void ROM_crcByte( unsigned char byte )
{
  if( !oddByte )
  {
    lowByte ^= byte;
  }
  else
  {
    highByte ^= byte;
  }
  oddByte ^= 0x01;
}

/*******************************************************************************
*
*******************************************************************************/
unsigned char ROM_getLowByte()
{
  return (lowByte^0xFF)&0xFF;
}

/*******************************************************************************
*
*******************************************************************************/
unsigned char ROM_getHighByte()
{
  return (highByte^0xFF)&0xFF;
}