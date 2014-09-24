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
#ifndef __COMMON_PAGES_H__
#define __COMMON_PAGES_H__                       

// Page structs
#define COMMON_PAGE_RESERVE_BYTE                           ((UCHAR) 0xFF)

typedef struct
{
   UCHAR ucPg80ReserveByte1;
   UCHAR ucPg80ReserveByte2;
   UCHAR  ucHwVersion;                               // Page 80
   USHORT usManId;                                   // Page 80
   USHORT usModelNumber;                             // Page 80
} CommonPage80_Data;

typedef struct
{
   UCHAR ucPg81ReserveByte1;
   UCHAR ucPg81ReserveByte2;
   UCHAR  ucSwVersion;                               // Page 81
   ULONG  ulSerialNumber;                            // Page 81
} CommonPage81_Data;

typedef struct
{
   UCHAR ucPg82ReserveByte1;
   UCHAR ucPg82ReserveByte2;
   ULONG ulCumOperatingTime;                        // Page 82
   UCHAR ucBattVoltage256;                          // Page 82
   UCHAR ucBattVoltage;                             // Page 82
   UCHAR ucBattStatus;                              // Page 82
   UCHAR ucCumOperatingTimeRes;                     // Page 82
   UCHAR ucDescriptiveField;					   //Used with WGT Display
} CommonPage82_Data;


#endif
