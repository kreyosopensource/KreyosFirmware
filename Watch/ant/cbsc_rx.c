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
#include "types.h"
#include "antinterface.h"
#include "antmessage.h"
#include "antdefines.h"
#include "serial.h"
#include "cbsc_rx.h"
#include "data.h"


///////////////////////////////////////////////////////////////////////////////
// ANT Network Setup definitions: combined bike speed and cadence
///////////////////////////////////////////////////////////////////////////////
// ANT Channel
#define CBSCRX_CHANNEL_TYPE           ((UCHAR) 0x00)     // Slave

// Channel ID
#define CBSCRX_DEVICE_TYPE            ((UCHAR) 0x79)

// Message Periods
#define CBSCRX_MSG_PERIOD             ((USHORT) 0x1F96)  // decimal 8086 (4.016Hz)

/////////////////////////////////////////////////////////////////////////////
// STATIC ANT Configuration Block
/////////////////////////////////////////////////////////////////////////////

BOOL HandleResponseEvents(UCHAR* pucBuffer_);
BOOL HandleDataMessages(UCHAR* pucBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_);


/////////////////////////////////////////////////////////////////////////////////////////
// Functon: CBSCRX_Init
//
// Descrption:
//
// Initialize all state variables.
// Params:
// N/A.
//
// returns: N/A.
/////////////////////////////////////////////////////////////////////////////////////////
void CBSCRX_Init()
{
   CBSCPage0_Data* pstCBSCPage0Data = CBSCRX_GetPage0();
   CBSCPage0_Data* pstCBSCPastPage0Data = CBSCRX_GetPastPage0();


   // Intialize app variables
   pstCBSCPage0Data->usLastTime1024 = 0;
   pstCBSCPage0Data->usCumSpeedRevCount = 0;
   pstCBSCPage0Data->usLastCadence1024 = 0;
   pstCBSCPage0Data->usCumCadenceRevCount = 0;
   pstCBSCPastPage0Data->usLastTime1024 = 0;
   pstCBSCPastPage0Data->usCumSpeedRevCount = 0;
   pstCBSCPastPage0Data->usLastCadence1024 = 0;
   pstCBSCPastPage0Data->usCumCadenceRevCount = 0;

}


/////////////////////////////////////////////////////////////////////////////////////////
// Functon: CBSCRX_Open
//
// Descrption:
// Opens CBSC recieve channel. Once the channel has been properly initialize an CBSCRX_EVENT_INIT_COMPLETE
// event will be generated via the callback. A positive response from this function does not
// indicate tha the channel successfully opened.
//
// Params:
// usDeviceNumber_: Device number to pair to. 0 for wild-card.
//
// returns: TRUE if all configuration messages were correctly setup and queued. FALSE otherwise.
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBSCRX_Open(UCHAR ucAntChannel_, USHORT usSearchDeviceNumber_, UCHAR usTransType_)
{
   CBSCRX_Init();

   ucAntChannel = ucAntChannel_;
   usDeviceNumber = usSearchDeviceNumber_;
   ucTransType = usTransType_;

   if (!ANT_AssignChannel(ucAntChannel,CBSCRX_CHANNEL_TYPE,ANTPLUS_NETWORK_NUMBER ))
      return FALSE;
   return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: CBSCRX_Close
//
// Description:
// Closes CBSC recieve channel and initializes all state variables. Once the channel has
// been successfuly closed, an CBSCRX_EVENT_CHANNEL_CLOSED event will be generated via the
// callback function/
//
// Params:
// N/A
//
// Returns: TRUE if message was successfully sent to ANT, FALSE otherwise.
//
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBSC_Close()
{
   return(ANT_CloseChannel(ucAntChannel));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: CBSCRX_GetPage0
//
// Descrption:
// Returns a pointer to the combined bike speed and cadence page 0 buffer. This function
// should be called following an CBSCRX_EVENT_PAGE 0 event to extract the latest data from
// page 0.
//
// Params:
// N/A
//
// returns: Pointer to combined CBSC Page 0 buffer, NULL if data not valid.
/////////////////////////////////////////////////////////////////////////////////////////
CBSCPage0_Data* CBSCRX_GetPage0()
{
   return &stCBSCPage0Data;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Functon: CBSCRX_GetPastPage0
//
// Descrption:
// Returns a pointer to the combined bike speed and cadence previous page 0 buffer. This
// function should be called when making instantaneous calculations.
//
// Params:
// N/A
//
// returns: Pointer to bike speed Page 0 buffer, NULL if data not valid.
/////////////////////////////////////////////////////////////////////////////////////////
CBSCPage0_Data* CBSCRX_GetPastPage0()
{
   return &stCBSCPastPage0Data;
}

//----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// CBSC_ChannelEvent
//
// Process channel event messages for the CBSC
//
// pucEventBuffer_: Pointer to ANT message buffer.
//
// \return: TRUE if buffer handled.
////////////////////////////////////////////////////////////////////////////
BOOL CBSCRX_ChannelEvent(UCHAR* pucEventBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   BOOL bTransmit = FALSE;
   UCHAR ucChannel = pucEventBuffer_[BUFFER_INDEX_CHANNEL_NUM] & 0x1F;
   pstEventStruct_->eEvent = ANTPLUS_EVENT_NONE;

   if(ucChannel == ucAntChannel)
   {
      if(pucEventBuffer_)
      {
         UCHAR ucANTEvent = pucEventBuffer_[BUFFER_INDEX_MESG_ID];
         switch( ucANTEvent )
         {
            case MESG_RESPONSE_EVENT_ID:
            {
               bTransmit = HandleResponseEvents(pucEventBuffer_);
               break;
            }
            case MESG_BROADCAST_DATA_ID:                    // Handle both BROADCAST, ACKNOWLEDGED and BURST data the same
            case MESG_ACKNOWLEDGED_DATA_ID:
            case MESG_BURST_DATA_ID:
            {
               bTransmit = HandleDataMessages(pucEventBuffer_, pstEventStruct_);
               break;
            }
            case MESG_CHANNEL_ID_ID:
            {
               usDeviceNumber = (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA];
               usDeviceNumber |= (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+1] << 8;
               ucTransType    = (UCHAR)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+3];

               pstEventStruct_->eEvent = ANTPLUS_EVENT_CHANNEL_ID;
               pstEventStruct_->usParam1 = usDeviceNumber;
               pstEventStruct_->usParam2 = ucTransType;
               break;
            }
         }
      }
   }

   return(bTransmit);
}

////////////////////////////////////////////////////////////////////////////
// HandleResponseEvents
//
// \return: N/A.
////////////////////////////////////////////////////////////////////////////
BOOL HandleResponseEvents(UCHAR* pucBuffer_)
{

   BOOL bTransmit = TRUE;

   if(pucBuffer_ && pucBuffer_[BUFFER_INDEX_RESPONSE_CODE] == RESPONSE_NO_ERROR)
   {
      switch(pucBuffer_[BUFFER_INDEX_RESPONSE_MESG_ID])
      {
         case MESG_ASSIGN_CHANNEL_ID:
         {
            ANT_ChannelId(ucAntChannel, usDeviceNumber,CBSCRX_DEVICE_TYPE, ucTransType );
            break;
         }
         case MESG_CHANNEL_ID_ID:
         {
            ANT_ChannelRFFreq(ucAntChannel, ANTPLUS_RF_FREQ);
            break;
         }
         case MESG_CHANNEL_RADIO_FREQ_ID:
         {
            ANT_ChannelPeriod(ucAntChannel, CBSCRX_MSG_PERIOD);
            break;
         }
         case MESG_CHANNEL_MESG_PERIOD_ID:
         {
            ANT_OpenChannel(ucAntChannel);
            break;
         }
         case MESG_OPEN_CHANNEL_ID:
         case MESG_CLOSE_CHANNEL_ID:            // Fallthrough
         default:
         {
            bTransmit = FALSE;                  // Can go back to sleep
         }
      }
   }
   return(bTransmit);
}

////////////////////////////////////////////////////////////////////////////
// HandleDataMessages
//
// \return: N/A.
////////////////////////////////////////////////////////////////////////////
BOOL HandleDataMessages(UCHAR* pucEventBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   BOOL bTransmit = FALSE;

   //page 0 combined sensor page utilizes all 8 bytes, and thus contains no page number; this byte
   //is non-definitive, as it is possible the first byte in the buffer corresponds to page 8x but
   //in fact is page 0 data, or an invalid page can me misconstrued as page 0 data by using 'default'
   stCBSCPage0Data.usLastCadence1024     = (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA];
   stCBSCPage0Data.usLastCadence1024    |= (USHORT)(pucEventBuffer_[BUFFER_INDEX_MESG_DATA+1] << 8);
   stCBSCPage0Data.usCumCadenceRevCount  = (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+2];
   stCBSCPage0Data.usCumCadenceRevCount |= (USHORT)(pucEventBuffer_[BUFFER_INDEX_MESG_DATA+3] << 8);
   stCBSCPage0Data.usLastTime1024        = (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+4];
   stCBSCPage0Data.usLastTime1024       |= (USHORT)(pucEventBuffer_[BUFFER_INDEX_MESG_DATA+5] << 8);
   stCBSCPage0Data.usCumSpeedRevCount    = (USHORT)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+6];
   stCBSCPage0Data.usCumSpeedRevCount   |= (USHORT)(pucEventBuffer_[BUFFER_INDEX_MESG_DATA+7] << 8);

   pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
   pstEventStruct_->usParam1 = CBSCRX_PAGE_0;

   if(usDeviceNumber == 0)
   {
      if(ANT_RequestMessage(ucAntChannel, MESG_CHANNEL_ID_ID))
         bTransmit = TRUE;
   }

   return(bTransmit);
}
