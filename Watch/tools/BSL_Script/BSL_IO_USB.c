/*==========================================================================*\
|                                                                            |
|                                                                            |
| PC-side Bootstrap Loader communication Application                         |
|                                                                            |
| See main.c for full version and legal information                          |
|                                                                            |
\*==========================================================================*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include "hiddevice.h"
#include "BSL_IO_USB.h"

int          USB_verbose = 0; /* verbose mode flag, default off */
struct strHidDevice device; /*HID device to use */

void USB_initializeCommPort();
/*******************************************************************************
*Function:    setVerbose
*Description: sets Verbose mode to on or off for debugging
*Parameters: 
*             int val   1 == verbose mode on, 0 == verbose mode off
*Returns:
*             none
*******************************************************************************/
void USB_setVerbose( unsigned int verb )
{
  USB_verbose = verb;
}

void USB_initialize_BSL(unsigned char* comPort)
{
  USB_initializeCommPort();

}

/*******************************************************************************
*Function:    initializeCommPort
*Description: initializes a COM port for communication
*Parameters: 
*             char *comPort         A string for the com port ie. "COM1"
*Returns:
*             none
*******************************************************************************/
void USB_initializeCommPort()
{
  HID_Init( &device );
  HID_Open( &device, 0x2047, 0x0200,  0 );
}

void USB_sendData( unsigned char* buf, unsigned int num_bytes )
{
  unsigned int i;
  unsigned char USB_buf[MAX_USB_FRAME_SIZE];
  //printf( "Step 1\n");
  memset( USB_buf, 0xAC, MAX_USB_FRAME_SIZE );
  
  //printf( "Step 2 %x @ %x\n", num_bytes, buf);
  USB_buf[0]= (char)num_bytes;
  num_bytes++;
  for( i = 1; i < num_bytes; i++)
  {
	//printf( "i = %x @ %x\n",i,&i );
	//printf( "writing %x to %x\n",buf[i-1],&USB_buf[i] );
    USB_buf[i] = buf[i-1];
  }
  
  //printf( "Step 3\n");
  //HID_WriteFile( &device, buf, size );
	if( USB_verbose )
	{
     int i;
	 for( i = 0; i< MAX_USB_FRAME_SIZE; i++ )
	 {
      printf( "{%2.2x} ",(USB_buf[i]&0xFF));
	 }
	}

   HID_WriteFile( &device, USB_buf, num_bytes+1 );
   //HID_WriteFile( &device, buf, num_bytes );
  return;
}

int USB_receiveData( unsigned char* buf )
{
  int retVal = 0;
  char buf2[MAX_USB_FRAME_SIZE+1];
  if ( HID_ReadFile( &device, buf2) == HID_DEVICE_SUCCESS)
  {
    int i;
	if( USB_verbose )
	{
        printf( "\n--------------------------------\n");
	}
	retVal = buf2[1];
	for( i = 0; i<= retVal; i++ )
	{
      buf[i] = buf2[i+2];

	}
	if( USB_verbose )
	{
	  for( i = 0; i< sizeof buf2; i++ )
	  {
        printf( "[%2.2x] ",(buf2[i]&0xFF));
	  }
	}
  }
  
  return retVal;

}

void USB_delay(int time) /* exported! */
{ 
#ifndef WIN32
  DWORD startTime= GetTickCount();
  while (calcTimeout(startTime) < time);
#else
  Sleep(time);
#endif
}