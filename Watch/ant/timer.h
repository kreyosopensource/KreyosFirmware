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
#ifndef TIMER_H
#define TIMER_H

#include "types.h"
                            
                            
                           
//////////////////////////////////////////////////////////////////////////////////
/// This delay must be at least 2.5us
/// So this must be modified for a specfic processor speed
//////////////////////////////////////////////////////////////////////////////////
#define TIMER_SRDY_PULSE_DELAY()    {__no_operation();__no_operation();__no_operation();__no_operation();\
                                     __no_operation();__no_operation();__no_operation();__no_operation();\
                                     __no_operation();__no_operation();__no_operation();__no_operation();} 
									/*{asm("nop");asm("nop");asm("nop");asm("nop");\
                                     asm("nop");asm("nop");asm("nop");asm("nop");\
                                     asm("nop");asm("nop");asm("nop");asm("nop");}*/



// Flags for course timer used for command timeouts and sensor simulator event
#define ALARM_FLAG_ONESHOT                ((UCHAR) 0x01)    // Timer runs once only
#define ALARM_FLAG_CONTINUOUS             ((UCHAR) 0x02)    // Timer will run continuously
#define ALARM_FLAG_ENABLED                ((UCHAR) 0x08)    // Alarm is enabled
#define ALARM_FLAG_BLINK0                 ((UCHAR) 0x10)    // Timer will force a blink of the LED on timer event
#define ALARM_FLAG_BLINK1                 ((UCHAR) 0x20)    // Timer will force a blink of the LED on timer event
#define ALARM_FLAG_BLINK2                 ((UCHAR) 0x40)    // Timer will force a blink of the LED on timer event
#define ALARM_FLAG_BLINK3                 ((UCHAR) 0x80)    // Timer will force a blink of the LED on timer event
#define ALARM_TIMER_PERIOD                ((USHORT) 1024)   // Rate at which timer will interrupt (per second)



typedef void (*EVENT_FUNC)(USHORT usTimeStamp_, UCHAR ucAlarmNumber_);
//////////////////////////////////////////////////////////////////////////////////
/// This delay must be at least 2.5us
/// So this must be modified for a specfic processor speed
//////////////////////////////////////////////////////////////////////////////////
void
Timer_Init(void);

void 
Timer_AssignTimerHandler(
   EVENT_FUNC pfTimeHandler_);

USHORT 
Timer_GetTime();

void 
Timer_DelayTime(
   USHORT usTimein10us);

UCHAR                                  // Alarm number (0 if alarm failed to get assigned)
Timer_RegisterAlarm(
   EVENT_FUNC pfCallback_,             // Pointer to function to call on alarm elapsed event
   UCHAR ucFlags_);                    // Alarm function flags

BOOL                                   // TRUE if alarm unregistered succesfully 
Timer_UnRegisterAlarm(
   UCHAR ucAlarmNumber_);              // Alarm number

BOOL                                   // Alarm started successfully
Timer_Start(
   UCHAR ucAlarmNumber_,               // Alarm number
   USHORT ucCount_);                   // Alarm elapsed count
   
USHORT                                 // 1024 tick count
Timer_GetTickCount();

void 
Timer_Stop(
   UCHAR ucAlarmNumber_);              // Alarm number



#endif
