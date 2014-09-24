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
#include "hrm_rx.h"
#include "antplus.h"
#include "data.h"


///////////////////////////////////////////////////////////////////////////////
// ANT Network Setup definitions
///////////////////////////////////////////////////////////////////////////////
// ANT Channel
#define HRMRX_CHANNEL_TYPE            ((UCHAR) 0x00)     // Slave

// Channel ID
#define HRMRX_DEVICE_TYPE             ((UCHAR) 0x78)	

// Message Periods
#define HRMRX_MSG_PERIOD              ((USHORT) 0x1F86)	// decimal 8070 (4.06Hz)

// HRM Page Toggle Bit Mask
#define TOGGLE_MASK                   ((UCHAR) 0x80)


/////////////////////////////////////////////////////////////////////////////
// STATIC ANT Configuration Block
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
   STATE_INIT_PAGE = 0,                                  // Initializing state
   STATE_STD_PAGE = 1,                                   // No extended messages to process
   STATE_EXT_PAGE = 2                                    // Extended messages to process
} StatePage;

static StatePage eThePageState;

// Local functions  
static void DecodeDefault(UCHAR* pucPayload_);
static BOOL HandleResponseEvents(UCHAR* pucBuffer_);
static BOOL HandleDataMessages(UCHAR* pucBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_);
/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_Init
//
// Descrption:
//
// Initialize all state variables. 
// Params:
// N/A.
//
// returns: N/A. 
/////////////////////////////////////////////////////////////////////////////////////////
void HRMRX_Init()
{
   HRMPage0_Data* pstPage0Data = HRMRX_GetPage0();
   HRMPage1_Data* pstPage1Data = HRMRX_GetPage1();
   HRMPage2_Data* pstPage2Data = HRMRX_GetPage2();
   HRMPage3_Data* pstPage3Data = HRMRX_GetPage3();
   HRMPage4_Data* pstPage4Data = HRMRX_GetPage4();
   
   // Intialize app variables
   pstPage0Data->usBeatTime = 0;                                  
   pstPage0Data->ucBeatCount = 0;                                 
   pstPage0Data->ucComputedHeartRate = 0; 
   pstPage1Data->ulOperatingTime = 0;
   pstPage2Data->ucManId = 0;
   pstPage2Data->ulSerialNumber = 0;
   pstPage3Data->ucHwVersion = 0;
   pstPage3Data->ucSwVersion = 0;
   pstPage3Data->ucModelNumber = 0;
   pstPage4Data->usPreviousBeat = 0;
   
   eThePageState = STATE_INIT_PAGE; 
   
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_Open
//
// Descrption:
// Opens HRM recieve channel. Once the channel has been properly initialize an HRMRX_EVENT_INIT_COMPLETE
// event will be generated via the callback. A positive response from this function does not
// indicate tha the channel successfully opened. 
//
// Params:
// usDeviceNumber_: Device number to pair to. 0 for wild-card.
//
// returns: TRUE if all configuration messages were correctly setup and queued. FALSE otherwise. 
/////////////////////////////////////////////////////////////////////////////////////////
BOOL HRMRX_Open(UCHAR ucAntChannel_, USHORT usSearchDeviceNumber_, UCHAR ucTransType_)
{

   HRMRX_Init();

   ucAntChannel = ucAntChannel_;
   usDeviceNumber = usSearchDeviceNumber_;
   ucTransType = ucTransType_;
   
   if(!ANT_AssignChannel(ucAntChannel,HRMRX_CHANNEL_TYPE,ANTPLUS_NETWORK_NUMBER ))
      return FALSE;
 
   return(TRUE);
} 

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_Close
//
// Description:
// Closes HRM recieve channel and initializes all state variables. Once the channel has 
// been successfuly closed, an HRMRX_EVENT_CHANNEL_CLOSED event will be generated via the
// callback function/
//
// Params:
// N/A
//
// Returns: TRUE if message was successfully sent to ANT, FALSE otherwise. 
// 
/////////////////////////////////////////////////////////////////////////////////////////
BOOL HRMRX_Close()
{
   ANT_CloseChannel(ucAntChannel);
   return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_GetPage0
//
// Descrption:
// Returns a pointer to the page 0 buffer. This function should be called following 
// an HRMRX_EVENT_PAGE 0 event to extract the latest data from page 0. 
//
// Params:
// N/A
//
// returns: Pointer to Page 0 buffer, NULL if data not valid.  
/////////////////////////////////////////////////////////////////////////////////////////
HRMPage0_Data* HRMRX_GetPage0()
{
   return &stPage0Data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_GetPage1
//
// Descrption:
// Returns a pointer to the page 1 buffer. This function should be called following 
// an HRMRX_EVENT_PAGE 1 event to extract the latest data from page 1. 
//
// Params:
// N/A
//
// returns: Pointer to Page 1 buffer, NULL if data not valid.  
/////////////////////////////////////////////////////////////////////////////////////////
HRMPage1_Data* HRMRX_GetPage1()
{
   return &stPage1Data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_GetPage2
//
// Descrption:
// Returns a pointer to the page 2 buffer. This function should be called following 
// an HRMRX_EVENT_PAGE 2 event to extract the latest data from page 2. 
//
// Params:
// N/A
//
// returns: Pointer to Page 2 buffer, NULL if data not valid.  
/////////////////////////////////////////////////////////////////////////////////////////
HRMPage2_Data* HRMRX_GetPage2()
{
   return &stPage2Data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_GetPage3
//
// Descrption:
// Returns a pointer to the page 3 buffer. This function should be called following 
// an HRMRX_EVENT_PAGE 3 event to extract the latest data from page 3. 
//
// Params:
// N/A
//
// returns: Pointer to Page 3 buffer, NULL if data not valid.  
/////////////////////////////////////////////////////////////////////////////////////////
HRMPage3_Data* HRMRX_GetPage3()
{
   return &stPage3Data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Functon: HRMRX_GetPage4
//
// Descrption:
// Returns a pointer to the page 4 buffer. This function should be called following 
// an HRMRX_EVENT_PAGE 4 event to extract the latest data from page 4. 
//
// Params:
// N/A
//
// returns: Pointer to Page 4 buffer, NULL if data not valid.  
/////////////////////////////////////////////////////////////////////////////////////////
HRMPage4_Data* HRMRX_GetPage4()
{
   return &stPage4Data;
}

//----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// HRM_ChannelEvent
//
// Process channel event messages for the HRM 
//
// pucEventBuffer_: Pointer to ANT message buffer.
//
// \return: TRUE if buffer handled. 
////////////////////////////////////////////////////////////////////////////
BOOL HRMRX_ChannelEvent(UCHAR* pucEventBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   BOOL bTransmit = TRUE;
   
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
               bTransmit = HandleResponseEvents( pucEventBuffer_ );
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
   return (bTransmit);
}

//----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// DecodeDefault
//
// Process channel event messages for the HRM 
//
// pucPayload_: Pointer to buffer contaning default HRM info only.
//
// \return: N/A. 
////////////////////////////////////////////////////////////////////////////
void DecodeDefault(UCHAR* pucPayload_)
{
   HRMPage0_Data* pstPage0Data = HRMRX_GetPage0();

   pstPage0Data->usBeatTime = (USHORT)pucPayload_[0];                  // Measurement time
   pstPage0Data->usBeatTime |= (USHORT)pucPayload_[1] << 8;
   pstPage0Data->ucBeatCount = (UCHAR)pucPayload_[2];                  // Measurement count
   pstPage0Data->ucComputedHeartRate = (USHORT)pucPayload_[3];         // Computed heart rate
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
            ANT_ChannelId(ucAntChannel, usDeviceNumber,HRMRX_DEVICE_TYPE, ucTransType );
            break;
         }
         case MESG_CHANNEL_ID_ID:
         {
            ANT_ChannelRFFreq(ucAntChannel, ANTPLUS_RF_FREQ);
            break;
         }
         case MESG_CHANNEL_RADIO_FREQ_ID:
         {
            ANT_ChannelPeriod(ucAntChannel, HRMRX_MSG_PERIOD);
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
BOOL HandleDataMessages(UCHAR* pucBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   BOOL bTransmit = FALSE;
   static UCHAR ucOldPage;
   
   UCHAR ucPage = pucBuffer_[BUFFER_INDEX_MESG_DATA];
   pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
   pstEventStruct_->usParam1 = HRM_PAGE_0;

   switch(eThePageState)
   {
      case STATE_INIT_PAGE:
      {
         eThePageState = STATE_STD_PAGE;               
         break;                  
      }
      case STATE_STD_PAGE:
      {

         // Check if the page if changing, if yes
         // then move to the next state, otherwise
         // only interpret page 0
         if(ucOldPage == ucPage)
            break;
         else
            eThePageState = STATE_EXT_PAGE;
   
         // INTENTIONAL FALLTHROUGH !!!
      }
      case STATE_EXT_PAGE:
      {
         switch(ucPage & ~TOGGLE_MASK)
         {
            case HRM_PAGE_1:
            {
               HRMPage1_Data* pstPage1Data = HRMRX_GetPage1();

               pstPage1Data->ulOperatingTime  = (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+1]; 
               pstPage1Data->ulOperatingTime |= (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+2] << 8; 
               pstPage1Data->ulOperatingTime |= (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 16; 
               pstPage1Data->ulOperatingTime *= 2;
      
               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_1;
               break;
            }
            case HRM_PAGE_2:
            {
               HRMPage2_Data* pstPage2Data = HRMRX_GetPage2();

               pstPage2Data->ucManId = pucBuffer_[BUFFER_INDEX_MESG_DATA + 1];
               pstPage2Data->ulSerialNumber  = (ULONG)usDeviceNumber;
               pstPage2Data->ulSerialNumber |= (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+2] << 16;
               pstPage2Data->ulSerialNumber |= (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 24;

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_2;
               break;                        
            }
            case HRM_PAGE_3:
            {
               HRMPage3_Data* pstPage3Data = HRMRX_GetPage3();

               pstPage3Data->ucHwVersion   = (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+1];
               pstPage3Data->ucSwVersion   = (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+2];
               pstPage3Data->ucModelNumber = (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+3];

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_3;
               break;
            }
            case HRM_PAGE_4:
            {
               HRMPage4_Data* pstPage4Data = HRMRX_GetPage4();

               pstPage4Data->usPreviousBeat  = (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+2];
               pstPage4Data->usPreviousBeat |= (ULONG)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 8;

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_4;
               break;
            }
            case HRM_PAGE_0:
            {
               // Handled above and below, so don't fall-thru to default case
               break;
            }
            default:
            {
               pstEventStruct_->eEvent = ANTPLUS_EVENT_UNKNOWN_PAGE;
               break;
            }
         }
         break;
      }
   }
   ucOldPage = ucPage;
   DecodeDefault( &pucBuffer_[BUFFER_INDEX_MESG_DATA+4] );  

   if(usDeviceNumber == 0)
   {
      if(ANT_RequestMessage(ucAntChannel, MESG_CHANNEL_ID_ID))
         bTransmit = TRUE;
   }
   
   return(bTransmit);
}


