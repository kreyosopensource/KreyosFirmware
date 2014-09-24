/////////////////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING EXAMPLE CODE IS INTENDED FOR LIMITED CIRCULATION ONLY.
// 
// Please forward all questions regarding this code to ANT Technical Support.
// 
// Dynastream Innovations Inc.
// 228 River Avenue
// Cochrane, Alberta, Canada
// T4C 2C1
// 
// (P) (403) 932-9292
// (F) (403) 932-6521
// (TF) 1-866-932-9292
// (E) support@thisisant.com
// 
// www.thisisant.com
//
// Reference Design Disclaimer
//
// The references designs and codes provided may be used with ANT devices only and remain the copyrighted property of 
// Dynastream Innovations Inc. The reference designs and codes are being provided on an "as-is" basis and as an accommodation, 
// and therefore all warranties, representations, or guarantees of any kind (whether express, implied or statutory) including, 
// without limitation, warranties of merchantability, non-infringement,
// or fitness for a particular purpose, are specifically disclaimed.
//
// ©2008 Dynastream Innovations Inc. All Rights Reserved
// This software may not be reproduced by
// any means without express written approval of Dynastream
// Innovations Inc.
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CBSC_RX__
#define __CBSC_RX__

#include "types.h"
#include "antinterface.h" 
#include "antplus.h"                  

// ANT Channel

typedef struct
{
   USHORT usLastCadence1024;                         // Page 0 CBSC
   USHORT usCumCadenceRevCount;                      // Page 0 CBSC
   USHORT usLastTime1024;                            // Page 0 CBSC
   USHORT usCumSpeedRevCount;                        // Page 0 CBSC
} CBSCPage0_Data;

void 
CBSCRX_Init();

BOOL 
CBSCRX_Open(                                         // TRUE is commands queued succesfully
   UCHAR ucAntChannel_,                              // ANT Channel
   USHORT usSearchDeviceNumber_,                     // ANT Channel device number
   UCHAR usTransType_);                              // ANT Channel transmission type
                                                     
BOOL 
CBSCRX_Close();                                      // TRUE if command queued succesfully 

BOOL                                                 // TRUE if commands queued to transmit (don't sleep transmit thread)
CBSCRX_ChannelEvent(                                  
   UCHAR* pucEventBuffer_,                           // Pointer to buffer containing response from ANT
   ANTPLUS_EVENT_RETURN* pstEventStruct_);           // Pointer to event structure set by this function
                                                     
CBSCPage0_Data*                                      // Pointer to page 0 data
CBSCRX_GetPage0();

CBSCPage0_Data*                                      // Pointer to previous page 0 data
CBSCRX_GetPastPage0();

#endif
