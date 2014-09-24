#include "timer.h"
#include "sys/ctimer.h"

#define MAX_TIMER 4
static struct ctimer timer[MAX_TIMER];
static EVENT_FUNC callbacks[MAX_TIMER];

static void callback(void *ptr)
{
  unsigned char i = (unsigned char)ptr;
  (*callbacks[i])(Timer_GetTickCount(), i);
}

UCHAR                                  // Alarm number (0 if alarm failed to get assigned)
Timer_RegisterAlarm(
                    EVENT_FUNC pfCallback_,             // Pointer to function to call on alarm elapsed event
                    UCHAR ucFlags_)                    // Alarm function flags
{
  unsigned int i;

  if (ALARM_FLAG_ONESHOT != ucFlags_)
	return 0;

  for(i = 0; i < MAX_TIMER; i++)
  {
    if (callbacks[i] == NULL)
    {
      callbacks[i] = pfCallback_;
      return i;
    }
  }

  return 0;
}

BOOL                                   // TRUE if alarm unregistered succesfully
Timer_UnRegisterAlarm(
                      UCHAR ucAlarmNumber_)              // Alarm number
{
  ctimer_stop(&timer[ucAlarmNumber_]);
  callbacks[ucAlarmNumber_] = NULL;

  return TRUE;
}

BOOL                                   // Alarm started successfully
Timer_Start(
            UCHAR ucAlarmNumber_,               // Alarm number
            USHORT ucCount_)                   // Alarm elapsed count
{
  ctimer_set(&timer[ucAlarmNumber_], ucCount_ >> 3, callback, (void*)ucAlarmNumber_);

  return TRUE;
}

USHORT                                 // 1024 tick count
Timer_GetTickCount()
{
  return clock_time() << 3;
}

void
Timer_Stop(
           UCHAR ucAlarmNumber_)              // Alarm number
{
  ctimer_stop(&timer[ucAlarmNumber_]);
}
