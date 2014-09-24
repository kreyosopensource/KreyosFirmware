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
#include "antdefines.h"
#include "timer.h"
#include "hrm_rx.h"
#include "cbsc_rx.h"
#include "ant.h"
#include "serial.h"

#include "contiki.h"
#include "window.h"
#include <stdio.h>
// ANT Channel settings
#define ANT_CHANNEL_HRMRX                          ((UCHAR) 0)          // Default ANT Channel
#define CBSCRX_ANT_CHANNEL                         ((UCHAR) 0)         // Default ANT Channel

// other defines
#define HRM_PRECISION                              ((ULONG)1000)

// other defines
#define CBSC_PRECISION                             ((ULONG)1000)

static const UCHAR aucNetworkKey[] = ANTPLUS_NETWORK_KEY;

static void ProcessANTHRMRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_);
static void ProcessAntEvents(UCHAR* pucEventBuffer_);
static void ProcessANTCBSCRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_);

ModeEnum mode;
/*----------Bicyle ------------------*/
//local variables
typedef struct
{
   ULONG  ulIntValue;
   USHORT usFracValue;
   USHORT usDeltaValue;
} RawValues;

static ULONG ulBSAccumRevs = 0;                    //initialize at zero
static ULONG ulBCAccumCadence = 0;                 //initialize at zero
static RawValues stSpeedData = {0};
static RawValues stCadenceData = {0};

//local functions
static void ComputeCadence(CBSCPage0_Data* pstPresent, CBSCPage0_Data* pstPast);
static void ComputeSpeed(CBSCPage0_Data* pstPresent, CBSCPage0_Data* pstPast);


PROCESS(ant_process, "ANT process");

void ant_process_poll()
{
  process_poll(&ant_process);
}

void ant_init(ModeEnum m)
{
  mode = m;
  ANTInterface_Init();

  process_start(&ant_process, NULL);
}

void ant_shutdown()
{
  Serial_Shutdown();

  process_exit(&ant_process);
}

//----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// main
//
// main function
//
// Configures device simulator and HRM TX channel.
//
// \return: This function does not return.
////////////////////////////////////////////////////////////////////////////
PROCESS_THREAD(ant_process, ev, data)
{
  static struct etimer timer;
  static UCHAR* pucRxBuffer;

  PROCESS_BEGIN();

  // wait about 500ms for ant module to start
  etimer_set(&timer, CLOCK_SECOND/2);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  ANT_Reset();

  // Main loop
  while(TRUE)
  {
    pucRxBuffer = ANTInterface_Transaction();                // Check if any data has been recieved from serial

    if(pucRxBuffer)
    {
      ANTPLUS_EVENT_RETURN stEventStruct;
      if (mode == MODE_HRM)
      {
        HRMRX_ChannelEvent(pucRxBuffer, &stEventStruct);
        ProcessANTHRMRXEvents(&stEventStruct);
      }
      else if (mode == MODE_CBSC)
      {
        CBSCRX_ChannelEvent(pucRxBuffer, &stEventStruct);
        ProcessANTCBSCRXEvents(&stEventStruct);
      }

      ProcessAntEvents(pucRxBuffer);
      ANTInterface_Complete();                              // Release the serial buffer

      continue;
    }

    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
  }

  PROCESS_END();
}


////////////////////////////////////////////////////////////////////////////
// ProcessANTCBSCRXEvents
//
// CBSC Reciever event processor
//
// Processes events recieved from CBSC module.
//
// \return: N/A
///////////////////////////////////////////////////////////////////////////
void ProcessANTCBSCRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_)
{
   switch (pstEvent_->eEvent)
   {

      case ANTPLUS_EVENT_CHANNEL_ID:
      {
         // Can store this device number for future pairings so that
         // wild carding is not necessary.
         printf("Device number is %d\n", pstEvent_->usParam1);
         printf("Transmission type is %d\n\n", pstEvent_->usParam2);
         break;
      }
      case ANTPLUS_EVENT_PAGE:
      {
         // Get data correspinding to the page. Only get the data you
         // care about.
         switch(pstEvent_->usParam1)
         {
            case CBSCRX_PAGE_0:
            {
               CBSCPage0_Data* pstPage0Data = CBSCRX_GetPage0();
               CBSCPage0_Data* pstPrev0Data = CBSCRX_GetPastPage0();

               printf("Combined Bike Speed and Cadence Page 0\n\n");

               ComputeSpeed(pstPage0Data, pstPrev0Data);
               ComputeCadence(pstPage0Data, pstPrev0Data);

               //update accumulated values, handles rollovers
               ulBSAccumRevs += (ULONG)((pstPage0Data->usCumSpeedRevCount - pstPrev0Data->usCumSpeedRevCount) & MAX_USHORT);
               ulBCAccumCadence += (ULONG)((pstPage0Data->usCumCadenceRevCount - pstPrev0Data->usCumCadenceRevCount) & MAX_USHORT);

#if 0
               printf("Current speed time: %u", (ULONG)(pstPage0Data->usLastTime1024 / 1024));
               printf(".%03u s\n", (ULONG)((((pstPage0Data->usLastTime1024 % 1024) * CBSC_PRECISION)+(ULONG)512)/(ULONG)1024));
               printf("Delta speed time: %u", (ULONG)(stSpeedData.usDeltaValue / 1024));
               printf(".%03u s\n", (ULONG)((((stSpeedData.usDeltaValue % 1024) * CBSC_PRECISION)+(ULONG)512)/(ULONG)1024));
               printf("Instantaneous time inverse (x circumference for inst speed): %u", (ULONG)(stSpeedData.ulIntValue));
               printf(".%03u 1/s\n", stSpeedData.usFracValue);
               printf("Accumulated revs (x circumference for total distance): 0x%04X", (USHORT)((ulBSAccumRevs >> 16) & MAX_USHORT));
               printf("%04X\n", (USHORT)(ulBSAccumRevs & MAX_USHORT)); //display limited by 16-bit CPU

               printf("Current cadence time: %u", (ULONG)(pstPage0Data->usLastCadence1024 / 1024));
               printf(".%03u s\n", (ULONG)((((pstPage0Data->usLastCadence1024 % 1024) * CBSC_PRECISION)+(ULONG)512)/(ULONG)1024));
               printf("Delta cadence time: %u", (ULONG)(stCadenceData.usDeltaValue / 1024));
               printf(".%03u s\n", (ULONG)((((stCadenceData.usDeltaValue % 1024) * CBSC_PRECISION)+(ULONG)512)/(ULONG)1024));
               printf("Instantaneous cadence: %u", (ULONG)(stCadenceData.ulIntValue));
               printf(".%03u RPM\n", stCadenceData.usFracValue);
               printf("Accumulated cadence: 0x%04X", (USHORT)((ulBCAccumCadence >> 16) & MAX_USHORT));
               printf("%04X\n\n\n", (USHORT)(ulBCAccumCadence & MAX_USHORT)); //display limited by 16-bit CPU
#endif
               window_postmessage(EVENT_SPORT_DATA, SPORTS_SPEED, (void*)(window_readconfig()->circumference * (stSpeedData.ulIntValue * CBSC_PRECISION + stSpeedData.usFracValue)));
               window_postmessage(EVENT_SPORT_DATA, SPORTS_CADENCE, (void*)stCadenceData.ulIntValue);

               //move current data to the past
               pstPrev0Data->usCumCadenceRevCount = pstPage0Data->usCumCadenceRevCount;
               pstPrev0Data->usLastCadence1024    = pstPage0Data->usLastCadence1024;
               pstPrev0Data->usCumSpeedRevCount   = pstPage0Data->usCumSpeedRevCount;
               pstPrev0Data->usLastTime1024       = pstPage0Data->usLastTime1024;

               break;
            }
            default:
            {
               // ASSUME PAGE 0
               printf("Invalid or undefined page\n\n");
               break;
            }
         }
         break;
      }
      case ANTPLUS_EVENT_UNKNOWN_PAGE:  // Decode unknown page manually
      case ANTPLUS_EVENT_NONE:
      default:
      {
         break;
      }
   }
}

////////////////////////////////////////////////////////////////////////////
// ProcessANTHRMRXEvents
//
// HRM Reciever event processor
//
// Processes events recieved from HRM module.
//
// \return: N/A
///////////////////////////////////////////////////////////////////////////
void ProcessANTHRMRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_)
{
  static UCHAR ucPreviousBeatCount = 0;

  switch (pstEvent_->eEvent)
  {

  case ANTPLUS_EVENT_CHANNEL_ID:
    {
      // Can store this device number for future pairings so that
      // wild carding is not necessary.
      printf("Device Number is %d\n", pstEvent_->usParam1);
      printf("Transmission type is %d\n\n", pstEvent_->usParam2);
      break;
    }
  case ANTPLUS_EVENT_PAGE:
    {
      HRMPage0_Data* pstPage0Data = HRMRX_GetPage0(); //common data
      BOOL bCommonPage = FALSE;

      //print formulated page identifier
      if (pstEvent_->usParam1 <= HRM_PAGE_4)
        printf("Heart Rate Monitor Page %d\n", pstEvent_->usParam1);

      // Get data correspinding to the page. Only get the data you
      // care about.
      switch(pstEvent_->usParam1)
      {
      case HRM_PAGE_0:
        {
          bCommonPage = TRUE;
          break;
        }
      case HRM_PAGE_1:
        {
          HRMPage1_Data* pstPage1Data = HRMRX_GetPage1();
          ULONG ulMinutes, ulHours, ulDays, ulSeconds;

          ulDays = (ULONG)((pstPage1Data->ulOperatingTime) / 86400);  //1 day == 86400s
          ulHours = (ULONG)((pstPage1Data->ulOperatingTime) % 86400); // half the calculation so far
          ulMinutes = ulHours % (ULONG)3600;
          ulSeconds = ulMinutes % (ULONG)60;
          ulHours /= (ULONG)3600; //finish the calculations: hours = 1hr == 3600s
          ulMinutes /= (ULONG)60; //finish the calculations: minutes = 1min == 60s

          //printf("Cumulative operating time: %dd ", ulDays);
          //printf("%dh ", ulHours);
          //printf("%dm ", ulMinutes);
          //printf("%ds\n\n", ulSeconds);
          bCommonPage = TRUE;
          break;
        }
      case HRM_PAGE_2:
        {
          HRMPage2_Data* pstPage2Data = HRMRX_GetPage2();

          //printf("Manufacturer ID: %u\n", pstPage2Data->ucManId);
          //printf("Serial No (upper 16-bits): 0x%X\n", pstPage2Data->ulSerialNumber);
          bCommonPage = TRUE;
          break;
        }
      case HRM_PAGE_3:
        {
          HRMPage3_Data* pstPage3Data = HRMRX_GetPage3();

          //printf("Hardware Rev ID %u ", pstPage3Data->ucHwVersion);
          //printf("Model %u\n", pstPage3Data->ucModelNumber);
          //printf("Software Ver ID %u\n", pstPage3Data->ucSwVersion);
          bCommonPage = TRUE;
          break;
        }
      case HRM_PAGE_4:
        {
          HRMPage4_Data* pstPage4Data = HRMRX_GetPage4();

          //printf("Previous heart beat event: %u.", (ULONG)(pstPage4Data->usPreviousBeat/1024));
          //printf("%03u s\n", (ULONG)((((pstPage4Data->usPreviousBeat % 1024) * HRM_PRECISION) + 512) / 1024));

          if((pstPage0Data->ucBeatCount - ucPreviousBeatCount) == 1)	// ensure that there is only one beat between time intervals
          {
            USHORT usR_RInterval = pstPage0Data->usBeatTime - pstPage4Data->usPreviousBeat;	// subtracting the event time gives the R-R interval
            //printf("R-R Interval: %u.", (ULONG)(usR_RInterval/1024));
            //printf("%03u s\n", (ULONG)((((usR_RInterval % 1024) * HRM_PRECISION) + 512) / 1024));
          }
          ucPreviousBeatCount = pstPage0Data->ucBeatCount;

          bCommonPage = TRUE;
          break;
        }
      default:
        {
          // ASSUME PAGE 0
          printf("Unknown format\n\n");
          break;
        }
      }
      if(bCommonPage)
      {
        window_postmessage(EVENT_SPORT_DATA, SPORTS_HEARTRATE, (void*)pstPage0Data->ucComputedHeartRate);
        //printf("Time of last heart beat event: %u.", (ULONG)(pstPage0Data->usBeatTime/1024));
        //printf("%03u s\n", (ULONG)((((pstPage0Data->usBeatTime % 1024) * HRM_PRECISION) + 512) / 1024));
        //printf("Heart beat count: %u\n", pstPage0Data->ucBeatCount);
        //printf("Instantaneous heart rate: %u bpm\n\n", pstPage0Data->ucComputedHeartRate);
      }
      break;
    }

  case ANTPLUS_EVENT_UNKNOWN_PAGE:  // Decode unknown page manually
  case ANTPLUS_EVENT_NONE:
  default:
    {
      break;
    }
  }
}

void ProcessAntEvents(UCHAR* pucEventBuffer_)
{

  if(pucEventBuffer_)
  {
    UCHAR ucANTEvent = pucEventBuffer_[BUFFER_INDEX_MESG_ID];
    switch( ucANTEvent )
    {
    case MESG_RESPONSE_EVENT_ID:
      {
        switch(pucEventBuffer_[BUFFER_INDEX_RESPONSE_CODE])
        {
        case EVENT_RX_SEARCH_TIMEOUT:
          {
            break;
          }
        case EVENT_TX:
          {
            break;
          }

        case RESPONSE_NO_ERROR:
          {
            if (pucEventBuffer_[3] == MESG_OPEN_CHANNEL_ID)
            {
              process_post(ui_process, EVENT_ANT_STATUS, (void*)BIT0);
            }
            else if (pucEventBuffer_[3] == MESG_CLOSE_CHANNEL_ID)
            {
            }
            else if (pucEventBuffer_[3] == MESG_NETWORK_KEY_ID)
            {
              if (mode == MODE_HRM)
              {
                //Once we get a response to the set network key
                //command, start opening the HRM channel
                HRMRX_Open(ANT_CHANNEL_HRMRX, 0, 0);
              }
              else
              {
                 CBSCRX_Open(CBSCRX_ANT_CHANNEL,0,0);
              }
            }
            break;
          }
        }
        break;
      }
    case MESG_STARTUP_ID:
      {
        printf("\n$$OK ANT\n");
        if (pucEventBuffer_[2] == 0)
        {
          printf("Reset is complete.\n");
        }
        else
        {
          if (pucEventBuffer_[2] & BIT0)
          {
            printf("HARDWARE_RESET_LINE ");
          }
          if (pucEventBuffer_[2] & BIT1)
          {
            printf("WATCH_DOG_RESET ");
          }
          if (pucEventBuffer_[2] & BIT5)
          {
            printf("COMMAND_RESET ");
          }
          if (pucEventBuffer_[2] & BIT6)
          {
            printf("SYNCHRONOUS_RESET ");
          }
          if (pucEventBuffer_[2] & BIT7)
          {
            printf(" SUSPEND_RESET ");
          }

          printf("\n");
        }
        ANT_NetworkKey(ANTPLUS_NETWORK_NUMBER, aucNetworkKey);
        break;
      }
    }
  }
}



static void ComputeCadence(CBSCPage0_Data* pstPresent, CBSCPage0_Data* pstPast)
{
   ULONG  ulFinalCadence;
   USHORT usDeltaTime;

   //rollover protection
   usDeltaTime = (pstPresent->usLastCadence1024 - pstPast->usLastCadence1024) & MAX_USHORT;

   if (usDeltaTime > 0) //divide by zero
   {
      //rollover protection
      ulFinalCadence = (ULONG)((pstPresent->usCumCadenceRevCount - pstPast->usCumCadenceRevCount) & MAX_USHORT);
      ulFinalCadence *= (ULONG)(60); //60 s/min for numerator

      stCadenceData.usFracValue = (USHORT)((((ulFinalCadence * 1024) % (ULONG)usDeltaTime) * CBSC_PRECISION) / usDeltaTime);
      ulFinalCadence = (ULONG)(ulFinalCadence * (ULONG)1024 / usDeltaTime); //1024/((1/1024)s) in the denominator --> RPM
                                                                            //...split up from s/min due to ULONG size limit

      stCadenceData.ulIntValue   = ulFinalCadence;
   }

   //maintain old data if time change not detected, but update delta time
   stCadenceData.usDeltaValue = usDeltaTime;

   return;
}

static void ComputeSpeed(CBSCPage0_Data* pstPresent, CBSCPage0_Data* pstPast)
{
   ULONG ulFinalSpeed;
   USHORT usDeltaTime;

   //rollover protection
   usDeltaTime = (pstPresent->usLastTime1024 - pstPast->usLastTime1024) & MAX_USHORT;

   if (usDeltaTime > 0) //divide by zero
   {
      //rollover protection
      ulFinalSpeed = (ULONG)((pstPresent->usCumSpeedRevCount - pstPast->usCumSpeedRevCount) & MAX_USHORT);
      ulFinalSpeed *= (ULONG)(1024); //circumference for numerator, 1024/((1/1024)s) in the denominator

      stSpeedData.usFracValue = (USHORT)(((ulFinalSpeed % (ULONG)usDeltaTime) * CBSC_PRECISION) / usDeltaTime);
      ulFinalSpeed /= (ULONG)(usDeltaTime);

      stSpeedData.ulIntValue   = ulFinalSpeed;
   }

   //maintain old data if time change not detected, but update delta time
   stSpeedData.usDeltaValue = usDeltaTime;

   return;
}