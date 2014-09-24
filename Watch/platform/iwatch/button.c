#include "contiki.h"
#include "button.h"
#include "window.h"
#include "system.h"
#include "sys/clock.h"
#include "sys/etimer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern void system_reset();

PROCESS(button_process, "Button Driver");

unsigned long downtime[4];
unsigned long eventtime[4];
struct etimer button_timer;

static inline uint8_t getmask(int i)
{
  if (i == 3)
    return 1 << 6;
  else
    return 1 << i;
}

void button_init()
{
  int i;
  process_start(&button_process, NULL);

  etimer_stop(&button_timer);
  for(i = 0; i < 4; i++)
  {
    uint8_t bitmask;

    bitmask = getmask(i);
    // INPUT
    P2DIR &= ~(bitmask);
    P2SEL &= ~(bitmask);

    // ENABLE INT
    P2IES |= bitmask;

    // pullup in dev board
    P2REN |= bitmask;
    P2OUT |= bitmask;
    P2IFG &= ~(bitmask);

    downtime[i] = 0;

    P2IE |= bitmask;
  }
}

int button_snapshot()
{
  int ret;
  ret = 0;
  for(int i = 0; i <3; i++)
  {
    if (!(P2IN & (getmask(i))))
      ret |= (1 << i);
  }
  return ret;
}

PROCESS_THREAD(button_process, ev, data)
{
  PROCESS_BEGIN();
  while(1)
  {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_POLL)
    {
      for(int i = 0; i < 4; i++)
      {
        uint8_t mask = getmask(i);

       // if ((P2IE & mask) == 0)
        {
          // need handle this button
          if (((P2IN & mask) == 0) && ((P2IES & mask) != 0)) // button is down
          {
            // key is down
            //process_post(ui_process, EVENT_KEY_DOWN, (void*)i);
            downtime[i] = eventtime[i]; 
            if (etimer_expired(&button_timer))
            {
              // first button
              etimer_set(&button_timer, CLOCK_SECOND/2);
            }
            P2IES ^= mask;
          }
          else if (((P2IN & mask) != 0) && ((P2IES & mask) == 0)) 
          {
            //process_post(ui_process, EVENT_KEY_UP, (void*)i);
            if (downtime[i] > 0)
            {
              if (eventtime[i] - downtime[i] > RTIMER_SECOND)
                process_post(ui_process, EVENT_KEY_LONGPRESSED, (void*)i);
              else
                process_post(ui_process, EVENT_KEY_PRESSED, (void*)i);
            }
            downtime[i] = 0;
            P2IES ^= mask;
          }
          else if (((P2IN & mask) == 0) && ((P2IES & mask) == 0)) // key is still down
          {
            if (etimer_expired(&button_timer))
            {
              // first button
              etimer_set(&button_timer, CLOCK_SECOND/2);
            }
          }
          else if (((P2IN & mask)) && ((P2IES & mask))) // key is still up
          {
            downtime[i] = 0;
          }

          P2IE |= mask;
        }
      }

    }
    else if (ev == PROCESS_EVENT_TIMER)
    {
      uint8_t reboot = 0;
      uint8_t downbutton = 0;
      uint8_t needquick = 0;
      for(int i = 0; i < 4; i++)
      {
        PRINTF("button %d downtime: %ld\n", i, downtime[i]);
        if (downtime[i] > 0)
        {
          downbutton++;
          if (downtime[i] < RTIMER_NOW() - RTIMER_SECOND * 3)
            reboot++;

          if (downtime[i] < RTIMER_NOW() - RTIMER_SECOND)
          {
            // check if we need fire another event
            process_post(ui_process, EVENT_KEY_PRESSED, (void*)i);
            needquick++;
          }
        }
      }

      PRINTF("%d buttons down, now=%ld\n", downbutton, RTIMER_NOW());

      if (reboot == 4)
        system_rebootToNormal();

      if (downbutton)
      {
        if (needquick)
          etimer_set(&button_timer, CLOCK_SECOND/4);
        else
          etimer_set(&button_timer, CLOCK_SECOND/2);
      }
      else
      {
          etimer_stop(&button_timer);
      }

    }
  }
  PROCESS_END();
}

static inline int port2_button(int i)
{
  P2IE &= ~(getmask(i));
  eventtime[i] = RTIMER_NOW();
  process_poll(&button_process);
  return 1;
}

int port2_pin0()
{
  return port2_button(0);
}

int port2_pin1()
{
  return port2_button(1);
}

int port2_pin2()
{
  return port2_button(2);
}

int port2_pin6()
{
  return port2_button(3);
}