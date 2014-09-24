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
#ifndef __ANT_PLUS_H__
#define __ANT_PLUS_H__

// Global ANT parameters
#define ANTPLUS_NETWORK_NUMBER               ((UCHAR) 0)
#define ANTPLUS_NETWORK_KEY                  { 0xB9,0xA5,0x21,0xFB,0xBD,0x72,0xC3,0x45 }
#define ANTPLUS_RF_FREQ                      ((UCHAR) 0x39)     // decimal 57 (2457 MHz) 

// SDM
#define SDM_PAGE_1                           ((UCHAR) 1)
#define SDM_PAGE_2                           ((UCHAR) 2)
#define SDM_PAGE_3                           ((UCHAR) 3)
#define SDM_PAGE_4                           ((UCHAR) 4)
#define SDM_PAGE_5                           ((UCHAR) 5)
#define SDM_PAGE_6                           ((UCHAR) 6)
#define SDM_PAGE_7                           ((UCHAR) 7)
#define SDM_PAGE_8                           ((UCHAR) 8)
#define SDM_PAGE_9                           ((UCHAR) 9)
#define SDM_PAGE_10                          ((UCHAR) 10)
#define SDM_PAGE_11                          ((UCHAR) 11)
#define SDM_PAGE_12                          ((UCHAR) 12)
#define SDM_PAGE_13                          ((UCHAR) 13)

// HRM
#define HRM_PAGE_0                           ((UCHAR) 0)
#define HRM_PAGE_1                           ((UCHAR) 1)
#define HRM_PAGE_2                           ((UCHAR) 2)
#define HRM_PAGE_3                           ((UCHAR) 3)
#define HRM_PAGE_4                           ((UCHAR) 4)

// COMBINED BIKE SPEED AND CADENCE
#define CBSCRX_PAGE_0                        ((UCHAR) 0)

// SEPARATE BIKE SPEED AND CADENCE
#define BSC_PAGE_0                           ((UCHAR) 0)
#define BSC_PAGE_1                           ((UCHAR) 1)
#define BSC_PAGE_2                           ((UCHAR) 2)
#define BSC_PAGE_3                           ((UCHAR) 3)

// BIKE POWER SENSOR
#define BPS_PAGE_1                           ((UCHAR) 1)
#define BPS_PAGE_16                          ((UCHAR) 16)
#define BPS_PAGE_17                          ((UCHAR) 17)
#define BPS_PAGE_18                          ((UCHAR) 18)
#define BPS_PAGE_32                          ((UCHAR) 32)

// WEIGHT SCALE
#define WEIGHT_PAGE_BODY_WEIGHT              ((UCHAR) 0x01)
#define WEIGHT_PAGE_BODY_COMPOSITION         ((UCHAR) 0x02)
#define WEIGHT_PAGE_METABOLIC_INFO           ((UCHAR) 0x03)
#define WEIGHT_PAGE_BODY_MASS                ((UCHAR) 0x04)
#define WEIGHT_PAGE_USER_PROFILE             ((UCHAR) 0x3A)
#define WEIGHT_PAGE_ANTFS_REQUEST            ((UCHAR) 0x46)
#define WEIGHT_COMMON_PAGE80				 ((UCHAR) 0x50)
#define WEIGHT_COMMON_PAGE81				 ((UCHAR) 0x51)
#define WEIGHT_COMMON_PAGE82				 ((UCHAR) 0x52)

// GLOBAL
#define GLOBAL_PAGE_80                       ((UCHAR) 80)
#define GLOBAL_PAGE_81                       ((UCHAR) 81)
#define GLOBAL_PAGE_82                       ((UCHAR) 82)

//Page 82 flags
#define GBL82_COARSE_BATT_INVALID_VOLTAGE          ((UCHAR) 0x0F)
#define GBL82_BATT_STATUS_NEW                      ((UCHAR) 1)
#define GBL82_BATT_STATUS_GOOD                     ((UCHAR) 2)
#define GBL82_BATT_STATUS_OK                       ((UCHAR) 3)
#define GBL82_BATT_STATUS_LOW                      ((UCHAR) 4)
#define GBL82_BATT_STATUS_CRITICAL                 ((UCHAR) 5)
#define GBL82_BATT_STATUS_INVALID                  ((UCHAR) 7)
#define GBL82_CUM_TIME_16_SECOND_RES               ((UCHAR) 0)
#define GBL82_CUM_TIME_2_SECOND_RES                ((UCHAR) 1)

typedef enum
{
   ANTPLUS_EVENT_NONE,              // No Event
   ANTPLUS_EVENT_INIT_COMPLETE,     // Initialization of channel complete
   ANTPLUS_EVENT_COMMAND_FAIL,      // A command has failed
   ANTPLUS_EVENT_CHANNEL_CLOSED,    // Closing of channel complete
   ANTPLUS_EVENT_CHANNEL_ID,        // Recieved channel ID information
   ANTPLUS_EVENT_PAGE,              // Got a page
   ANTPLUS_EVENT_UNKNOWN_PAGE,      // Got unknown page (could not decode)
   ANTPLUS_EVENT_TRANSMIT,           // Transmitted a page
   ANTPLUS_EVENT_CALIBRATION_TIMEOUT // Calibration request timed out
}ANTPLUS_Events;

typedef struct
{
   ANTPLUS_Events eEvent;           // Event
   USHORT usParam1;                 // Generic parameter 1
   USHORT usParam2;                 // Generic parameter 2
} ANTPLUS_EVENT_RETURN;

#endif
