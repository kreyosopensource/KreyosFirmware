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
#ifndef __ANTINTERFACE_H__
#define __ANTINTERFACE_H__

#include "types.h"
#include "antmessage.h"

typedef struct
{
   UCHAR ucControl;
   UCHAR aucData[ANT_DATA_SIZE];
} ANT_MESG_STRUCT;

typedef union
{
   ANT_MESG_STRUCT stMessageStruct;
   UCHAR aucMesg[sizeof(ANT_MESG_STRUCT)];
} ANT_MESG;

void 
ANTInterface_Init();

UCHAR* 
ANTInterface_Transaction();

void 
ANTInterface_Complete();

BOOL 
ANT_Reset();

BOOL 
ANT_UnAssignChannel(
   UCHAR ucChannelNumber_);

BOOL 
ANT_AssignChannel(
   UCHAR ucChannelNumber_, 
   UCHAR ucChannelType_, 
   UCHAR ucNetworkNumber_);

BOOL 
ANT_SearchTimeout(
   UCHAR ucChannelNumber_, 
   UCHAR ucTimeout_);


BOOL 
ANT_NetworkKey(
   UCHAR ucNetworkNumber_, 
   const UCHAR* pucKey_);
   
BOOL 
ANT_ChannelId(
   UCHAR ucANTChannel_, 
   USHORT usDeviceNumber_, 
   UCHAR ucDeviceType_, 
   UCHAR ucTransmitType_);
   
BOOL 
ANT_ChannelPower(
   UCHAR ucANTChannel_, 
   UCHAR ucPower_);
   
BOOL 
ANT_ChannelRFFreq(
   UCHAR ucANTChannel_,
   UCHAR ucFreq_);
   
BOOL 
ANT_ChannelPeriod(
   UCHAR ucANTChannel_, 
   USHORT usPeriod_);
   
BOOL 
ANT_Broadcast(
   UCHAR ucANTChannel_, 
   UCHAR* pucBuffer_);
      
BOOL 
ANT_AcknowledgedTimeout(
   UCHAR ucChannel_, 
   UCHAR* pucData_, 
   USHORT usTimeout_);
   
BOOL 
ANT_Acknowledged(
   UCHAR ucANTChannel_, 
   UCHAR* pucBuffer_);
   
BOOL 
ANT_BurstPacket(
   UCHAR ucControl_, 
   UCHAR* pucBuffer_);
   
USHORT 
ANT_SendBurstTransfer(
   UCHAR ucAntChannel_, 
   UCHAR* pucBuffer_, 
   USHORT usPackets_);
   
USHORT 
ANT_SendPartialBurst(
   UCHAR ucAntChannel_,
   UCHAR* pucBuffer_, 
   USHORT usPackets_, 
   ULONG ulInitialPacket_, 
   BOOL bIncludeLast_);
   
BOOL 
ANT_OpenChannel(
   UCHAR ucANTChannel_);
   
BOOL 
ANT_CloseChannel(
   UCHAR ucANTChannel_);
   
BOOL 
ANT_RequestMessage(
   UCHAR ucANTChannel_, 
   UCHAR ucRequestedMessage_);
   
BOOL 
ANT_RunScript(
   UCHAR ucPageNum_);
   
   
#endif
