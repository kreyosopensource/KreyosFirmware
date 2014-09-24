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
#ifndef __SERIAL_H___
#define __SERIAL_H__

#include "types.h"

BOOL
Serial_Init();

void
Serial_Shutdown();

BOOL
Serial_SyncReset(void);

void
Serial_Transaction(void);

UCHAR*
Serial_Get_Tx_Buffer();

void
Serial_Put_Tx_Buffer();

UCHAR*
Serial_Get_Rx_Buffer();

void
Serial_Put_Rx_Buffer();

UCHAR*
Serial_Read_Rx_Top();
void
Serial_Release_Rx_Top();

UCHAR*
Serial_Read_Rx_Buffer();

void
Serial_Release_Rx_Buffer();

void
Serial_Flush_Tx();

void
Serial_Flush_Rx();

int
Asynchronous_WriteByte(
   int iByte);

#endif
