#include "contiki.h"
#include "backlight.h"
#include "sys/ctimer.h"

static struct ctimer light_timer;
static struct ctimer motor_timer;
void backlight_init()
{
  LIGHTDIR |= LIGHT;
  LIGHTSEL |= LIGHT;

  MOTORSEL |= MOTOR;
  MOTORDIR |= MOTOR;

  TB0CTL |= TBSSEL_1 + MC_1;
  TB0CCR0 = 32; // control PWM freq = 32768/16 = 2048hz

  MOTORCONTROL = OUTMOD_0;
  LIGHTCONTROL = OUTMOD_0;
}

void backlight_shutdown()
{
  MOTORCONTROL = OUTMOD_0;
  LIGHTCONTROL = OUTMOD_0;
}

void light_stop(void *ptr)
{
  if (LIGHTLEVEL > 2)
  {
    LIGHTLEVEL--;
    clock_time_t length = (clock_time_t)ptr;
    ctimer_set(&light_timer, length, light_stop, (void*)length);
  }
  else
    LIGHTCONTROL = OUTMOD_0;
}

void backlight_on(uint8_t level, clock_time_t length)
{
  if (level > 8) level = 8;

  level *= 2;
  if (level == 0)
  {
    LIGHTCONTROL = OUTMOD_0;
  }
  else
  {
    LIGHTCONTROL = OUTMOD_7;
    LIGHTLEVEL = level * 2;
    if (length > 0)
      ctimer_set(&light_timer, length, light_stop, (void*)(CLOCK_SECOND/8));
  }
}

void motor_stop(void *ptr)
{
  MOTORCONTROL = OUTMOD_0;
}

void motor_on(uint8_t level, clock_time_t length)
{
  if (level > 16) level = 16;
  level *= 2;

  if (level == 0)
  {
    motor_stop(NULL);
  }
  else
  {
    MOTORCONTROL = OUTMOD_7;
    MOTORLEVEL = level;
    if (length > 0)
      ctimer_set(&motor_timer, length, motor_stop, NULL);
  }
}
