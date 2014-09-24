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
#include "BSL_USB_Protocol_5xx.h"
#include "BSL_IO_USB.h"


unsigned char USB_5xx_TXPacket( dataBuffer db )
{
  USB_sendData( db.data, db.size );
  return 0;
}

unsigned char USB_5xx_RXPacket( dataBuffer* db )
{
  db->size = USB_receiveData( db->data );
  return 0;
}
