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

#ifndef ANTDEFINES_H
#define ANTDEFINES_H

#include "types.h"

//////////////////////////////////////////////
// ANT Clock Definition
//////////////////////////////////////////////
#define ANT_CLOCK_FREQUENCY                        ((ULONG)32768)          ///< ANT system clock frequency

//////////////////////////////////////////////
// Radio TX Power Definitions
//////////////////////////////////////////////
#define RADIO_TX_POWER_MASK                        ((UCHAR)0x03)
#define RADIO_TX_POWER_MINUS20DB                   ((UCHAR)0x00)
#define RADIO_TX_POWER_MINUS10DB                   ((UCHAR)0x01)
#define RADIO_TX_POWER_MINUS5DB                    ((UCHAR)0x02)
#define RADIO_TX_POWER_0DB                         ((UCHAR)0x03)

//////////////////////////////////////////////
// Default System Definitions
//////////////////////////////////////////////
#define DEFAULT_CHANNEL_MESSAGE_FREQUENCY          ((ULONG)4)
#define DEFAULT_CHANNEL_MESSAGE_PERIOD             ((USHORT)( ANT_CLOCK_FREQUENCY / DEFAULT_CHANNEL_MESSAGE_FREQUENCY )) ///< 8192 (4Hz)
#define DEFAULT_RADIO_TX_POWER                     RADIO_TX_POWER_0DB      ///< ANT default RF power
#define DEFAULT_RADIO_CHANNEL                      ((UCHAR)66)             ///< 2400MHz + 1MHz * Channel Number = 2466MHz
#define DEFAULT_RX_SEARCH_TIMEOUT                  ((UCHAR)12)             ///< 12 * 2.5 seconds = 30 seconds

//////////////////////////////////////////////
// ID Definitions
//////////////////////////////////////////////
#define ID_MANUFACTURER_OFFSET                     ((UCHAR)3)
#define ID_DEVICE_TYPE_OFFSET                      ((UCHAR)2)
#define ID_DEVICE_NUMBER_HIGH_OFFSET               ((UCHAR)1)
#define ID_DEVICE_NUMBER_LOW_OFFSET                ((UCHAR)0)
#define ID_DEVICE_TYPE_PAIRING_FLAG                ((UCHAR)0x80)

//////////////////////////////////////////////
// Assign Channel Parameters
//////////////////////////////////////////////
#define PARAMETER_TX_NOT_RX                        ((UCHAR)0x10)
#define PARAMETER_MULTIPLE_ACCESS_CHANNEL          ((UCHAR)0x20)  
#define PARAMETER_NO_TX_GUARD_BAND                 ((UCHAR)0x40)   
#define PARAMETER_ALWAYS_RX_WILD_CARD_SEARCH_ID    ((UCHAR)0x40)

//////////////////////////////////////////////
// Assign Channel Types
//////////////////////////////////////////////
#define CHANNEL_TYPE_SLAVE                         ((UCHAR) 0x00)
#define CHANNEL_TYPE_MASTER                        ((UCHAR) 0x10)
#define CHANNEL_TYPE_MASTER_TX_ONLY                ((UCHAR) 0x50)
#define CHANNEL_TYPE_SHARED_SLAVE                  ((UCHAR) 0x20)
#define CHANNEL_TYPE_SHARED_MASTER                 ((UCHAR) 0x30)

//////////////////////////////////////////////
// Channel Status
//////////////////////////////////////////////
#define STATUS_UNASSIGNED_CHANNEL                  ((UCHAR)0x00)
#define STATUS_ASSIGNED_CHANNEL                    ((UCHAR)0x01)
#define STATUS_SEARCHING_CHANNEL                   ((UCHAR)0x02)
#define STATUS_TRACKING_CHANNEL                    ((UCHAR)0x03)
#define STATUS_OVERRUN                             ((UCHAR)0x40)
#define STATUS_UNDERRUN                            ((UCHAR)0x80)

//////////////////////////////////////////////
// Standard capabilities defines
//////////////////////////////////////////////
#define CAPABILITIES_NO_RX_CHANNELS                ((UCHAR)0x01)
#define CAPABILITIES_NO_TX_CHANNELS                ((UCHAR)0x02)
#define CAPABILITIES_NO_RX_MESSAGES                ((UCHAR)0x04)
#define CAPABILITIES_NO_TX_MESSAGES                ((UCHAR)0x08)
#define CAPABILITIES_NO_ACKD_MESSAGES              ((UCHAR)0x10)
#define CAPABILITIES_NO_BURST_TRANSFER             ((UCHAR)0x20)

//////////////////////////////////////////////
// Advanced capabilities defines
//////////////////////////////////////////////
#define CAPABILITIES_DETECT_OVERRUN_UNDERRUN       ((UCHAR)0x01)
#define CAPABILITIES_NETWORK_ENABLED               ((UCHAR)0x02)

//////////////////////////////////////////////
// Burst Message Sequence 
//////////////////////////////////////////////
#define CHANNEL_NUMBER_MASK                        ((UCHAR)0x1F)
#define SEQUENCE_NUMBER_MASK                       ((UCHAR)0xE0)
#define SEQUENCE_NUMBER_INC                        ((UCHAR)0x20)
#define SEQUENCE_NUMBER_ROLLOVER                   ((UCHAR)0x60)
#define SEQUENCE_LAST_MESSAGE                      ((UCHAR)0x80)

//////////////////////////////////////////////
// Shared Channel Commands / Datatypes
//////////////////////////////////////////////
#define SHARED_CMD_SLOT_AVALIBLE                   ((UCHAR)0xFF)
#define SHARED_CMD_BUSY_ACQUIRING                  ((UCHAR)0xFE)
#define SHARED_CMD_COMMAND_REQUEST_TO_ACQUIRE      ((UCHAR)0xFD)
#define SHARED_CMD_CONFIRM_ACQUIRED                ((UCHAR)0xFC)
#define SHARED_CMD_NO_SLOTS_AVAILABLE              ((UCHAR)0xFB)
//...
#define SHARED_TYPE_RELAY                          ((UCHAR)0x43)           
#define SHARED_TYPE_COUNTER                        ((UCHAR)0x42)
#define SHARED_TYPE_A_TO_D                         ((UCHAR)0x41)
#define SHARED_TYPE_DIGITAL                        ((UCHAR)0x40)           
#define SHARED_TYPE_UNDEFINED                      ((UCHAR)0x00)

///////////////////////////////////////////////////////////////////////
// AtoD SubTypes
///////////////////////////////////////////////////////////////////////
#define TEMPERATURE                                ((UCHAR)0xFE)
#define BATT_VOLTAGE                               ((UCHAR)0xFF)

//////////////////////////////////////////////
// Response / Event Codes
//////////////////////////////////////////////
#define RESPONSE_NO_ERROR                          ((UCHAR)0x00)             

#define EVENT_RX_SEARCH_TIMEOUT                    ((UCHAR)0x01)             
#define EVENT_RX_FAIL                              ((UCHAR)0x02)             
#define EVENT_TX                                   ((UCHAR)0x03)             
#define EVENT_TRANSFER_RX_FAILED                   ((UCHAR)0x04)             
#define EVENT_TRANSFER_TX_COMPLETED                ((UCHAR)0x05)             
#define EVENT_TRANSFER_TX_FAILED                   ((UCHAR)0x06)             
#define EVENT_CHANNEL_CLOSED                       ((UCHAR)0x07)
#define EVENT_RX_FAIL_GO_TO_SEARCH                 ((UCHAR)0x08)
#define EVENT_CHANNEL_COLLISION                    ((UCHAR)0x09)


#define CHANNEL_IN_WRONG_STATE                     ((UCHAR)0x15)           // returned on attempt to perform an action from the wrong channel state
#define CHANNEL_NOT_OPENED                         ((UCHAR)0x16)           // returned on attempt to communicate on a channel that is not open
#define CHANNEL_ID_NOT_SET                         ((UCHAR)0x18)           // returned on attempt to open a channel without setting the channel ID

#define TRANSFER_IN_PROGRESS                       ((UCHAR)0x1F)           // returned on attempt to communicate on a channel with a TX transfer in progress
#define TRANSFER_SEQUENCE_NUMBER_ERROR             ((UCHAR)0x20)           // returned when sequence number is out of order on a Burst transfer

#define INVALID_MESSAGE                            ((UCHAR)0x28)           // returned when the message has an invalid parameter
#define INVALID_NETWORK_NUMBER                     ((UCHAR)0x29)           // returned when an invalid network number is provided
#define NO_RESPONSE_MESSAGE                        ((UCHAR)0x50)           // returned to the Command_SerialMessageProcess function, so no reply message is generated

#define FIT_ACTIVE_SEARCH_TIMEOUT                  ((UCHAR)0x60)           // event added for timeout of the pairing state after the Fit module becomes active
#define FIT_WATCH_PAIRED                           ((UCHAR)0x61)           // event added for timeout of the pairing state after the Fit module becomes active
#define FIT_WATCH_UNPAIRED                         ((UCHAR)0x62)           // event added for timeout of the pairing state after the Fit module becomes active


#define EVENT_COMMAND_TIMEOUT                      ((UCHAR)0xA9)           // (Host Only)returned when no response is recieved from ANT module after a command has been sent
#define EVENT_ACK_TIMEOUT                          ((UCHAR)0xAA)           // (Host Only) returned if not response to an Ack command is recieved for a set period of time.



///////////////////////////////////////////////////////////////
// Application Level defines
///////////////////////////////////////////////////////////////
#define USE_FREQUENCY_HOPPER                       
// #define USE_AUTO_SHARED_MASTER                     

#define NUM_CHANNELS                               ((UCHAR)0x04)           // Number of channels to be initialized and used by the application
#define NUM_FREQUENCY_HOPS                         ((UCHAR)0x04)           // Number of frequency hops to make if using frequency hopper

///////////////////////////////////////////////////////////////
// +LINK Mode Commands
///////////////////////////////////////////////////////////////
#define PLUS_LINK_MSG_STATUS         ((UCHAR)0x01)
#define PLUS_LINK_MSG_CONNECT        ((UCHAR)0x02)
#define PLUS_LINK_MSG_DISCONNECT     ((UCHAR)0x03)
#define PLUS_LINK_MSG_AUTHENTICATE   ((UCHAR)0x04)
#define PLUS_LINK_MSG_PAIR           ((UCHAR)0x05)
#define PLUS_LINK_MSG_DOWNLOAD       ((UCHAR)0x06)
#define PLUS_LINK_MSG_UPLOAD         ((UCHAR)0x07)
#define PLUS_LINK_MSG_ERASE          ((UCHAR)0x08)

#define TRANSFER_BUSY                              ((UCHAR)0x22) 

#define CHANNEL_0						0
///////////////////////////////////////////////////////////////
// State Machine Return Flags
///////////////////////////////////////////////////////////////
#define STATE_STATUS_NONE						((UCHAR)0x00) // State machine did not handle event in any way
#define STATE_STATUS_TRANSMIT             ((UCHAR)0x01) // State machine requires tx buffer to be transmitted. 
#define STATE_STATUS_HANDLED              ((UCHAR)0x02) // State machine handled input events, no further processing required 




//////////////////////////////////////////////
// PC Application Event Codes
//////////////////////////////////////////////
//NOTE: These events are not generated by the embedded ANT module

#define EVENT_RX_BROADCAST                         ((UCHAR)0x9A)           // returned when module receives broadcast data
#define EVENT_RX_ACKNOWLEDGED                      ((UCHAR)0x9B)           // returned when module receives acknowledged data
#define EVENT_RX_BURST_PACKET                      ((UCHAR)0x9C)           // returned when module receives burst data

#define EVENT_RX_EXT_BROADCAST                     ((UCHAR)0x9D)           // returned when module receives broadcast data
#define EVENT_RX_EXT_ACKNOWLEDGED                  ((UCHAR)0x9E)           // returned when module receives acknowledged data
#define EVENT_RX_EXT_BURST_PACKET                  ((UCHAR)0x9F)           // returned when module receives burst data

#define EVENT_RX_RSSI_BROADCAST                    ((UCHAR)0xA0)           // returned when module receives broadcast data
#define EVENT_RX_RSSI_ACKNOWLEDGED                 ((UCHAR)0xA1)           // returned when module receives acknowledged data
#define EVENT_RX_RSSI_BURST_PACKET                 ((UCHAR)0xA2)           // returned when module receives burst data

#define EVENT_RX_BTH_BROADCAST                     ((UCHAR)0xA3)           // returned when module receives broadcast data
#define EVENT_RX_BTH_ACKNOWLEDGED                  ((UCHAR)0xA4)           // returned when module receives acknowledged data
#define EVENT_RX_BTH_BURST_PACKET                  ((UCHAR)0xA5)           // returned when module receives burst data

#define EVENT_RX_BTH_EXT_BROADCAST                 ((UCHAR)0xA6)           // returned when module receives broadcast data
#define EVENT_RX_BTH_EXT_ACKNOWLEDGED              ((UCHAR)0xA7)           // returned when module receives acknowledged data
#define EVENT_RX_BTH_EXT_BURST_PACKET              ((UCHAR)0xA8)           // returned when module receives burst data
//////////////////////////////////////////////
// NVM Command Codes
//////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////
#define HIGH_BYTE(usWord) (UCHAR)((usWord >> 8) & 0x00FF) 
#define LOW_BYTE(usWord)  (UCHAR)(usWord & 0x00FF) 
#define BYTE0(x)                      ((UCHAR) x & 0xFF)
#define BYTE1(x)                      ((UCHAR) (x >> 8) & 0xFF)
#define BYTE2(x)                      ((UCHAR) (x >> 16) & 0xFF)
#define BYTE3(x)                      ((UCHAR) (x >> 24) & 0xFF)
#define MIN(x,y)        (((x)<(y))?(x):(y))
#define MAX(x,y)        (((x)>(y))?(x):(y))


#endif // !ANTDEFINES_H                                                    


