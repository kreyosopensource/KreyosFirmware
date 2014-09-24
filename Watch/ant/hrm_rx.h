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
#ifndef __HRM_RX__
#define __HRM_RX__

#include "types.h"
#include "antinterface.h"
#include "antplus.h"                   
#include "antinterface.h"                   


// Page structs
typedef struct
{
   USHORT usBeatTime;                                // All pages
   UCHAR ucBeatCount;                                // All pages
   UCHAR ucComputedHeartRate;                        // All pages
} HRMPage0_Data;

typedef struct
{
   ULONG ulOperatingTime;                            // Page 1
} HRMPage1_Data;

typedef struct
{
   UCHAR ucManId;                                    // Page 2
   ULONG ulSerialNumber;                             // Page 2
} HRMPage2_Data;

typedef struct
{
   UCHAR ucHwVersion;                                // Page 3
   UCHAR ucSwVersion;                                // Page 3
   UCHAR ucModelNumber;                              // Page 3
} HRMPage3_Data;

typedef struct
{
   UCHAR ucManufSpecific;                            // Page 4
   USHORT usPreviousBeat;                            // Page 4
} HRMPage4_Data;

// Public Functions
void 
HRMRX_Init();

BOOL                                                 // TRUE is commands queued succesfully    
HRMRX_Open(
   UCHAR ucAntChannel_,                              // ANT Channel number
   USHORT usSearchDeviceNumber_,                     // ANT Channel device number
   UCHAR ucTransType_);                              // ANT Channel transmission type
   
BOOL                                                 // TRUE if command queued succesfully  
HRMRX_Close();

BOOL                                                 // TRUE if commands queued to transmit (don't sleep transmit thread)
HRMRX_ChannelEvent(
   UCHAR* pucEventBuffer_,                           // Pointer to buffer containing response from ANT
   ANTPLUS_EVENT_RETURN* pstEventStruct_);           // Pointer to event structure set by this function

HRMPage0_Data*                                       // Pointer to page 0 data
HRMRX_GetPage0();

HRMPage1_Data*                                       // Pointer to page 1 data
HRMRX_GetPage1();

HRMPage2_Data*                                       // Pointer to page 2 data
HRMRX_GetPage2();                                                       

HRMPage3_Data*                                       // Pointer to page 3 data
HRMRX_GetPage3();

HRMPage4_Data*                                       // Pointer to page 4 data
HRMRX_GetPage4();

#endif
