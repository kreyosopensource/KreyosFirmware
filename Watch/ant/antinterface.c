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
#include "serial.h"
#include "antmessage.h"
#include "antdefines.h"
#include "antinterface.h"
#include "timer.h"
#include "serial.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define MAX_ACK_ALARMS_CHANNELS  ((UCHAR) 2)       // So only channels 0 and 1 can have ack timeouts, which are relevent only for a slave channel anyway
                                                   // Can always expand this to include all channels if necessary
typedef struct  
{
   UCHAR ucChannel;
   UCHAR ucMessageID;
   UCHAR ucResponseID;
   BOOL bWaitingForResponse;
   USHORT usTimeout;
}TimeoutStruct;

typedef struct
{
   UCHAR ucAlarmNumber;                       
   TimeoutStruct stTimeoutStruct;            
}TimeoutAlarmStruct;                            


static TimeoutAlarmStruct stCommandTimeout;                          // Command timeout struct
static TimeoutAlarmStruct astAckTimeouts[MAX_ACK_ALARMS_CHANNELS];   // Alarm Number associated with this channel               
                                                                     // Just implement two simulataneus timeout message at a time.
                                                                     // This can be changed to suit the application.

// Local Funcs
static void ProcessAntEvents(UCHAR* pucBuffer_);
static void WaitForResponse(TimeoutAlarmStruct* pclTimeoutStruct_, UCHAR ucChannel_, UCHAR ucResponseID_, UCHAR ucMessageID_);
static void ClearWait(TimeoutAlarmStruct* pclTimeoutStruct_);
static void ProcessAntEvents(UCHAR* pucBuffer_);
static void CommandTimeout(USHORT usTime1024_, UCHAR ucAlarmNumber_);
static void CommandAckTimeout(USHORT usTime1024_, UCHAR ucAlarmNumber_);

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// ANTInterface_Init
//
// This function initialize the ANTInterface module
//
////////////////////////////////////////////////////////////////////////////////
void ANTInterface_Init()
{
   UCHAR ucIndex;

   Serial_Init();              
   
   // Register a dedicated timer for commands 
   stCommandTimeout.ucAlarmNumber = Timer_RegisterAlarm( CommandTimeout, ALARM_FLAG_ONESHOT);
   stCommandTimeout.stTimeoutStruct.usTimeout = ALARM_TIMER_PERIOD/2;   // 500ms
   
   for (ucIndex = 0; ucIndex < MAX_ACK_ALARMS_CHANNELS; ucIndex++)
   {
      astAckTimeouts[ucIndex].ucAlarmNumber = 0;
      ClearWait(&astAckTimeouts[ucIndex]);
   }                  
   
   
   //!! ASSERT ucAlarmNumber does not equal 0        
   ClearWait(&stCommandTimeout);
}

////////////////////////////////////////////////////////////////////////////////
// ANTInterface_Transaction
//
//
////////////////////////////////////////////////////////////////////////////////
UCHAR* ANTInterface_Transaction()
{
   UCHAR* pucRxBuffer;
   
   // Transmit and recieve messages to/from ANT
   Serial_Transaction();
   
   pucRxBuffer = Serial_Read_Rx_Buffer();                // Check if any data has been recieved from serial

   ProcessAntEvents(pucRxBuffer);
   
   return(pucRxBuffer);
}

////////////////////////////////////////////////////////////////////////////////
// ANTInterface_Complete
//
//
////////////////////////////////////////////////////////////////////////////////
void ANTInterface_Complete()
{
   Serial_Release_Rx_Buffer();
}



////////////////////////////////////////////////////////////////////////////////
// ANT_Reset
//
// Note!! This command does not have a response from ANT, however, we want to use the 
// time out event to trigger when it is safe to send other commands again. Because
// the command timeout is 500ms, this is enough time to determine that the ANT
// MCU has reset. 
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_Reset()
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_SYSTEM_RESET_SIZE;
         pucBuffer[1] = MESG_SYSTEM_RESET_ID;
         pucBuffer[2] = 0;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,0, MESG_STARTUP_ID, 0);
   }
   return(bSuccess); 

}




////////////////////////////////////////////////////////////////////////////////
// ANT_AssignChannel
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_AssignChannel(UCHAR ucChannelNumber_, UCHAR ucChannelType_, UCHAR ucNetworkNumber_)
{
   BOOL bSuccess = FALSE;
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_ASSIGN_CHANNEL_SIZE;
         pucBuffer[1] = MESG_ASSIGN_CHANNEL_ID;
         pucBuffer[2] = ucChannelNumber_;
         pucBuffer[3] = ucChannelType_;
         pucBuffer[4] = ucNetworkNumber_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      
      WaitForResponse(&stCommandTimeout,ucChannelNumber_, MESG_RESPONSE_EVENT_ID, MESG_ASSIGN_CHANNEL_ID);

   }
   //!! Else ASSERT


   
   return(bSuccess);
}

////////////////////////////////////////////////////////////////////////////////
// ANT_UnAssignChannel
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_UnAssignChannel(UCHAR ucChannelNumber_)
{
   BOOL bSuccess = FALSE;
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_UNASSIGN_CHANNEL_SIZE;
         pucBuffer[1] = MESG_UNASSIGN_CHANNEL_ID;
         pucBuffer[2] = ucChannelNumber_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      
      WaitForResponse(&stCommandTimeout,ucChannelNumber_, MESG_RESPONSE_EVENT_ID, MESG_UNASSIGN_CHANNEL_ID);

   }
   //!! Else ASSERT
   return(bSuccess);
}

////////////////////////////////////////////////////////////////////////////////
// ANT_SearchTimeout
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_SearchTimeout(UCHAR ucChannelNumber_, UCHAR ucTimeout_)
{
   BOOL bSuccess = FALSE;
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_CHANNEL_SEARCH_TIMEOUT_SIZE;
         pucBuffer[1] = MESG_CHANNEL_SEARCH_TIMEOUT_ID;
         pucBuffer[2] = ucChannelNumber_;
         pucBuffer[3] = ucTimeout_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      
      WaitForResponse(&stCommandTimeout,ucChannelNumber_, MESG_RESPONSE_EVENT_ID, MESG_CHANNEL_SEARCH_TIMEOUT_ID);

   }
   //!! Else ASSERT


   
   return(bSuccess);
}

////////////////////////////////////////////////////////////////////////////////
// ANT_NetworkKey
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_NetworkKey(UCHAR ucNetworkNumber_, const UCHAR* pucKey_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_NETWORK_KEY_SIZE;
         pucBuffer[1] = MESG_NETWORK_KEY_ID;
         pucBuffer[2] = ucNetworkNumber_;
         pucBuffer[3] = pucKey_[0];
         pucBuffer[4] = pucKey_[1];
         pucBuffer[5] = pucKey_[2];
         pucBuffer[6] = pucKey_[3];
         pucBuffer[7] = pucKey_[4];
         pucBuffer[8] = pucKey_[5];
         pucBuffer[9] = pucKey_[6];
         pucBuffer[10] = pucKey_[7];
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,0, MESG_RESPONSE_EVENT_ID, MESG_NETWORK_KEY_ID);
   }
   return(bSuccess);

   
}

////////////////////////////////////////////////////////////////////////////////
// ANT_ChannelId
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_ChannelId(UCHAR ucANTChannel_, USHORT usDeviceNumber_, UCHAR ucDeviceType_, UCHAR ucTransmitType_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_CHANNEL_ID_SIZE;
         pucBuffer[1] = MESG_CHANNEL_ID_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = LOW_BYTE(usDeviceNumber_);
         pucBuffer[4] = HIGH_BYTE(usDeviceNumber_);
         pucBuffer[5] = ucDeviceType_;
         pucBuffer[6] = ucTransmitType_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_CHANNEL_ID_ID);
   }
   return(bSuccess);
   
}

/////////////////////////////////////////////////////////////////////////
// Priority: 
//
// ucPower_:   0 = TX Power -20dBM
//             1 = TX Power -10dBM
//             2 = TX Power -5dBM
//             3 = TX Power 0dBM
//
/////////////////////////////////////////////////////////////////////////
BOOL ANT_ChannelPower(UCHAR ucANTChannel_, UCHAR ucPower_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_RADIO_TX_POWER_SIZE;
         pucBuffer[1] = MESG_RADIO_TX_POWER_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucPower_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_RADIO_TX_POWER_ID);
   }
   return(bSuccess);
}


////////////////////////////////////////////////////////////////////////////////
// ANT_ChannelRFFreq
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_ChannelRFFreq(UCHAR ucANTChannel_,UCHAR ucFreq_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_CHANNEL_RADIO_FREQ_SIZE;
         pucBuffer[1] = MESG_CHANNEL_RADIO_FREQ_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucFreq_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_CHANNEL_RADIO_FREQ_ID);
   }
   return(bSuccess);
}



////////////////////////////////////////////////////////////////////////////////
// ANT_ChannelPeriod
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_ChannelPeriod(UCHAR ucANTChannel_, USHORT usPeriod_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_CHANNEL_MESG_PERIOD_SIZE;
         pucBuffer[1] = MESG_CHANNEL_MESG_PERIOD_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = LOW_BYTE(usPeriod_);
         pucBuffer[4] = HIGH_BYTE(usPeriod_);
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_CHANNEL_MESG_PERIOD_ID);
   } 
   return(bSuccess); 
}

////////////////////////////////////////////////////////////////////////////////
// ANT_Broadcast
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_Broadcast(UCHAR ucANTChannel_, UCHAR* pucBuffer_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(pucBuffer)                                         // If there is space in the queue
   {
      pucBuffer[0] = MESG_DATA_SIZE;
      pucBuffer[1] = MESG_BROADCAST_DATA_ID;
      pucBuffer[2] = ucANTChannel_;
      pucBuffer[3] = pucBuffer_[0];
      pucBuffer[4] = pucBuffer_[1];
      pucBuffer[5] = pucBuffer_[2];
      pucBuffer[6] = pucBuffer_[3];
      pucBuffer[7] = pucBuffer_[4];
      pucBuffer[8] = pucBuffer_[5];
      pucBuffer[9] = pucBuffer_[6];
      pucBuffer[10] = pucBuffer_[7];
      Serial_Put_Tx_Buffer();                         // Put buffer back in queue
      bSuccess = TRUE; 
   }
   
   return(bSuccess); 

}

////////////////////////////////////////////////////////////////////////////////
// ANT_Acknowledged
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_Acknowledged(UCHAR ucANTChannel_, UCHAR* pucBuffer_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(pucBuffer)                                         // If there is space in the queue
   {
      pucBuffer[0] = MESG_DATA_SIZE;
      pucBuffer[1] = MESG_ACKNOWLEDGED_DATA_ID;
      pucBuffer[2] = ucANTChannel_;
      pucBuffer[3] = pucBuffer_[0];
      pucBuffer[4] = pucBuffer_[1];
      pucBuffer[5] = pucBuffer_[2];
      pucBuffer[6] = pucBuffer_[3];
      pucBuffer[7] = pucBuffer_[4];
      pucBuffer[8] = pucBuffer_[5];
      pucBuffer[9] = pucBuffer_[6];
      pucBuffer[10] = pucBuffer_[7];
      Serial_Put_Tx_Buffer();                         // Put buffer back in queue
      bSuccess = TRUE; 
   }
   
   return(bSuccess); 

}


////////////////////////////////////////////////////////////////////////////////
// ANT_AcknowledgedTimeout
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_AcknowledgedTimeout(UCHAR ucChannel_, UCHAR* pucData_, USHORT usTimeout_)
{
	PRINTF("%s\n", __FUNCTION__);
   if(ucChannel_ >= MAX_ACK_ALARMS_CHANNELS)
      usTimeout_ = 0;

   if(usTimeout_ && !astAckTimeouts[ucChannel_].stTimeoutStruct.bWaitingForResponse)
   {
      ULONG ulTimeout = ALARM_TIMER_PERIOD*(ULONG)usTimeout_;
      ulTimeout /= 1000;
      
      // Register a dedicated timer for commands 
      astAckTimeouts[ucChannel_].ucAlarmNumber = Timer_RegisterAlarm( CommandAckTimeout, ALARM_FLAG_ONESHOT);
      astAckTimeouts[ucChannel_].stTimeoutStruct.usTimeout = (USHORT)ulTimeout;  

      WaitForResponse(&astAckTimeouts[ucChannel_], ucChannel_, MESG_RESPONSE_EVENT_ID, MESG_ACKNOWLEDGED_DATA_ID);
      
   }
   return(ANT_Acknowledged(ucChannel_, pucData_));
}





////////////////////////////////////////////////////////////////////////////////
// ANT_BurstPacket
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_BurstPacket(UCHAR ucControl_, UCHAR* pucBuffer_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(pucBuffer)                                         // If there is space in the queue
   {
      pucBuffer[0] = MESG_DATA_SIZE;
      pucBuffer[1] = MESG_BURST_DATA_ID;
      pucBuffer[2] = ucControl_;
      pucBuffer[3] = pucBuffer_[0];
      pucBuffer[4] = pucBuffer_[1];
      pucBuffer[5] = pucBuffer_[2];
      pucBuffer[6] = pucBuffer_[3];
      pucBuffer[7] = pucBuffer_[4];
      pucBuffer[8] = pucBuffer_[5];
      pucBuffer[9] = pucBuffer_[6];
      pucBuffer[10] = pucBuffer_[7];
      Serial_Put_Tx_Buffer();                         // Put buffer back in queue
      bSuccess = TRUE; 
   }
   
   return(bSuccess); 
}

////////////////////////////////////////////////////////////////////////////////
// ANT_SendBurstTransfer
//
//
////////////////////////////////////////////////////////////////////////////////
USHORT ANT_SendBurstTransfer(UCHAR ucAntChannel_, UCHAR* pucBuffer_, USHORT usPackets_)
{

   UCHAR ucSeq = 0;
	PRINTF("%s\n", __FUNCTION__);
   do
   {
      if (usPackets_ == 1)
         ucSeq |= SEQUENCE_LAST_MESSAGE;


      if(ANT_BurstPacket(ucAntChannel_ | ucSeq, pucBuffer_))
      {

         //Move to next 8 byte block in data
         pucBuffer_ += MESG_DATA_SIZE - 1;  //Use size - 1 since the channel data is not in the data buffer
         //Adjust sequence number
         if (ucSeq == SEQUENCE_NUMBER_ROLLOVER)
            ucSeq = SEQUENCE_NUMBER_INC;
         else
            ucSeq += SEQUENCE_NUMBER_INC;
      }

      // Transmit and recieve messages to/from ANT
      Serial_Transaction();


   }while(--usPackets_);

   return usPackets_;
}

////////////////////////////////////////////////////////////////////////////////
// ANT_SendPartialBurst
//
// usInitialPacket_ = number of initial packet within the burst sequence (eg. 3 for the third packet)
// bIncludeLast_ = FALSE if the last sequence number is not to be included in this block, TRUE otherwise
//
////////////////////////////////////////////////////////////////////////////////
USHORT ANT_SendPartialBurst(UCHAR ucAntChannel_, UCHAR* pucBuffer_, USHORT usPackets_, ULONG ulInitialPacket_, BOOL bIncludeLast_)
{
	PRINTF("%s\n", __FUNCTION__);
   UCHAR ucSeq;
   
   // Get initial sequence number
   if(ulInitialPacket_ <= 4)
      ucSeq = (ulInitialPacket_ - 1) * SEQUENCE_NUMBER_INC;
   else
      ucSeq = (1 + ((ulInitialPacket_ + 1) % 3)) * SEQUENCE_NUMBER_INC;     // adjust for rollover

   do
   {
      if (usPackets_ == 1 && bIncludeLast_)
         ucSeq |= SEQUENCE_LAST_MESSAGE;

      if(ANT_BurstPacket(ucSeq, pucBuffer_))
      {
         //Move to next 8 byte block in data
          pucBuffer_ += MESG_DATA_SIZE - 1;  //Use size - 1 since the channel data is not in the data buffer
         //Adjust sequence number
         if (ucSeq == SEQUENCE_NUMBER_ROLLOVER)
            ucSeq = SEQUENCE_NUMBER_INC;
         else
            ucSeq += SEQUENCE_NUMBER_INC;
      }

      // Transmit and recieve messages to/from ANT
      Serial_Transaction();

   }while(--usPackets_);

   return usPackets_;
}

////////////////////////////////////////////////////////////////////////////////
// ANT_OpenChannel
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_OpenChannel(UCHAR ucANTChannel_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
   
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_OPEN_CHANNEL_SIZE;
         pucBuffer[1] = MESG_OPEN_CHANNEL_ID;
         pucBuffer[2] = ucANTChannel_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_OPEN_CHANNEL_ID);
   } 
   
   return(bSuccess); 
}

////////////////////////////////////////////////////////////////////////////////
// ANT_CloseChannel
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_CloseChannel(UCHAR ucANTChannel_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_CLOSE_CHANNEL_SIZE;
         pucBuffer[1] = MESG_CLOSE_CHANNEL_ID;
         pucBuffer[2] = ucANTChannel_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, MESG_RESPONSE_EVENT_ID, MESG_CLOSE_CHANNEL_ID);
   }
   
   return(bSuccess); 
      
}

////////////////////////////////////////////////////////////////////////////////
// ANT_RequestMessage
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_RequestMessage(UCHAR ucANTChannel_, UCHAR ucRequestedMessage_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_REQUEST_SIZE;
         pucBuffer[1] = MESG_REQUEST_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucRequestedMessage_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,ucANTChannel_, ucRequestedMessage_, MESG_REQUEST_ID);
   }
   
   return(bSuccess); 
   
}

////////////////////////////////////////////////////////////////////////////////
// ANT_RunScript
//
//
////////////////////////////////////////////////////////////////////////////////
BOOL ANT_RunScript(UCHAR ucPageNum_)
{
   BOOL bSuccess = FALSE;
   UCHAR* pucBuffer = Serial_Get_Tx_Buffer();            // Get space from the queue
   PRINTF("%s\n", __FUNCTION__);
   if(!stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
      if(pucBuffer)                                         // If there is space in the queue
      {
         pucBuffer[0] = MESG_RUN_SCRIPT_SIZE;
         pucBuffer[1] = MESG_RUN_SCRIPT_ID;
         pucBuffer[2] = 0;
         pucBuffer[3] = ucPageNum_;
         Serial_Put_Tx_Buffer();                         // Put buffer back in queue
         bSuccess = TRUE; 
      }
      WaitForResponse(&stCommandTimeout,0, MESG_RESPONSE_EVENT_ID, MESG_RUN_SCRIPT_ID);
   }
   
   return(bSuccess); 
}





////////////////////////////////////////////////////////////////////////////////
// ProcessAntEvents
//
//
////////////////////////////////////////////////////////////////////////////////
void ProcessAntEvents(UCHAR* pucBuffer_)
{
   UCHAR ucChannel = pucBuffer_[2];     // Only makes sense if message has channel info obviously

   if(pucBuffer_)
   {
   
      if(stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
      {
         UCHAR ucMessageCode = pucBuffer_[BUFFER_INDEX_MESG_ID];   
   
         if(ucMessageCode == MESG_RESPONSE_EVENT_ID)
         {
            if(pucBuffer_[3] == stCommandTimeout.stTimeoutStruct.ucMessageID)
            {
               ClearWait(&stCommandTimeout);
            }
         }
         else if(ucMessageCode == stCommandTimeout.stTimeoutStruct.ucResponseID)
         {
            ClearWait(&stCommandTimeout);
         }
      }
   
   
      if(astAckTimeouts[ucChannel].stTimeoutStruct.bWaitingForResponse)
      {
         switch(pucBuffer_[4])
         {   
            case EVENT_TRANSFER_TX_COMPLETED:
            case EVENT_TRANSFER_TX_FAILED:
            case EVENT_CHANNEL_CLOSED:
            case CHANNEL_NOT_OPENED:
            case TRANSFER_IN_PROGRESS:
            case CHANNEL_IN_WRONG_STATE:
            {
               ClearWait(&astAckTimeouts[ucChannel]);
               Timer_Stop( astAckTimeouts[ucChannel].ucAlarmNumber );        
               Timer_UnRegisterAlarm( astAckTimeouts[ucChannel].ucAlarmNumber );
            
               break;
            }
            default:
               break;
         } 
      }
   }
}




/**************************************************************************
 * WaitForResponse
 * 
 * Sets state variables and starts timer. Called when a command is sent
 * that requires some response. 
 *
 * Params: 
 *
 * ucChannel_: ANT Channel
 * ucResponseID_: Message ID of the response we are waiting for
 * ucMessageID_: The message ID of the message just sent (the one istigating the response)
 *
 * 
 * returns: Right away
 *
 **************************************************************************/      
void WaitForResponse(TimeoutAlarmStruct* pclTimeoutStruct_, UCHAR ucChannel_, UCHAR ucResponseID_, UCHAR ucMessageID_)
{  

   pclTimeoutStruct_->stTimeoutStruct.ucMessageID = ucMessageID_;
   pclTimeoutStruct_->stTimeoutStruct.ucResponseID = ucResponseID_;
   pclTimeoutStruct_->stTimeoutStruct.ucChannel = ucChannel_;

   pclTimeoutStruct_->stTimeoutStruct.bWaitingForResponse = TRUE;

   Timer_Start( pclTimeoutStruct_->ucAlarmNumber, pclTimeoutStruct_->stTimeoutStruct.usTimeout);    // Start the command timeout timer         
}


/**************************************************************************
 * ClearWait
 * 
 * Clears the waiting state machines variables. This should be called if waiting 
 * for a response to some command and it is recieved, or if the request times out.
 * 
 *
 * Params: N/A
 * 
 * returns: N/A
 *
 **************************************************************************/      
void ClearWait(TimeoutAlarmStruct* pclTimeoutStruct_)
{  
   pclTimeoutStruct_->stTimeoutStruct.bWaitingForResponse = FALSE;
   pclTimeoutStruct_->stTimeoutStruct.ucMessageID = 0;
   pclTimeoutStruct_->stTimeoutStruct.ucResponseID = 0;
   pclTimeoutStruct_->stTimeoutStruct.ucChannel = 0;
}


/**************************************************************************
 * CommmandTimeout
 * 
 * Params: N/A
 * 
 * returns: N/A
 *
 **************************************************************************/      
void CommandTimeout(USHORT usTime1024_, UCHAR ucAlarmNumber_)
{
   if(stCommandTimeout.stTimeoutStruct.bWaitingForResponse)
   {
   
      UCHAR* pucBuffer_;
      
      // Get a buffer from the serial queue to fake out an ANT 
      // message
      pucBuffer_ =  Serial_Get_Rx_Buffer();
      
      if(pucBuffer_)
      {
      
         pucBuffer_[0] = 3;
         pucBuffer_[1] = MESG_RESPONSE_EVENT_ID;
         pucBuffer_[2] = stCommandTimeout.stTimeoutStruct.ucChannel;
         pucBuffer_[3] = stCommandTimeout.stTimeoutStruct.ucMessageID;
         pucBuffer_[4] = EVENT_COMMAND_TIMEOUT;
      
         // Since the RESET command does not have a response, 
         // fake one out after a timeout on the host MCU.
         if(stCommandTimeout.stTimeoutStruct.ucMessageID == MESG_SYSTEM_RESET_ID)
         {
            pucBuffer_[4] = RESPONSE_NO_ERROR;   
         }
      
      
      
         Serial_Put_Rx_Buffer();          // Put this message on the ANT recieve queue
   
      }
      
      ClearWait(&stCommandTimeout);
      
   }
   // else assert
}

/**************************************************************************
 * CommmandAckTimeout
 * 
 * Params: N/A
 * 
 * returns: N/A
 *
 **************************************************************************/      
void CommandAckTimeout(USHORT usTime1024_, UCHAR ucAlarmNumber_)
{
   UCHAR ucIndex;
   
   for (ucIndex = 0; ucIndex < MAX_ACK_ALARMS_CHANNELS; ucIndex++)
   {
      if(astAckTimeouts[ucIndex].ucAlarmNumber == ucAlarmNumber_)
         break;
   }                  
                  
   if(ucIndex < MAX_ACK_ALARMS_CHANNELS && astAckTimeouts[ucIndex].stTimeoutStruct.bWaitingForResponse)
   {
   
      UCHAR* pucBuffer_;
      
      // Get a buffer from the serial queue to fake out an ANT 
      // message
      pucBuffer_ =  Serial_Get_Rx_Buffer();
      
      if(pucBuffer_)
      {
         pucBuffer_[0] = 3;
         pucBuffer_[1] = MESG_RESPONSE_EVENT_ID;
         pucBuffer_[2] = astAckTimeouts[ucIndex].stTimeoutStruct.ucChannel;
         pucBuffer_[3] = astAckTimeouts[ucIndex].stTimeoutStruct.ucMessageID;
         pucBuffer_[4] = EVENT_ACK_TIMEOUT;
      
      
         Serial_Put_Rx_Buffer();          // Put this message on the ANT recieve queue
   
      }
      
      // For ack timeouts always clear them out as they are one shot.
      ClearWait(&astAckTimeouts[ucIndex]);
      Timer_Stop( astAckTimeouts[ucIndex].ucAlarmNumber);        
      Timer_UnRegisterAlarm( astAckTimeouts[ucIndex].ucAlarmNumber);
      
   }
   // else assert
   
   
   
}



















