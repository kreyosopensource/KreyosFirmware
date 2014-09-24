#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "memlcd.h"
#include "rtc.h"
#include <stdio.h>

#define MAX_STOP 15 // memory.h has hardcoded 15
#include "memory.h"
    
#define  state d.stop.state
#define  times d.stop.times
#define  counter d.stop.counter
#define  currentStop d.stop.currentStop
#define  topView  d.stop.topView
#define  saved_times d.stop.saved_times
#define delta_times d.stop.delta_times 

enum _STATE
{
  STATE_RUNNING = 0,
  STATE_STOP,
  STATE_INIT
};

#define NUMBASE 78

static void OnDraw(tContext* pContext)
{
  // clear the screen
  GrContextForegroundSet(pContext, ClrBlack);
  GrContextBackgroundSet(pContext, ClrWhite);
  GrRectFill(pContext, &client_clip);

  // draw the countdown time
  GrContextFontSet(pContext, &g_sFontGothic28b);
  window_drawtime(pContext, 45, times, 0);

  if ((state != STATE_INIT) && (pContext->sClipRegion.sYMax > 70))
  {
    GrContextFontSet(pContext, &g_sFontGothic18b);
    //printf("stop: %d current: %d\n", currentStop, topView);
    char buf[20];

    // draw the stoped times
    for(int i = topView; (i < topView + 3) && (i < currentStop); i++)
    {
      sprintf(buf, "%02d", i + 1);
      GrStringDraw(pContext, buf, -1, 2, (i - topView) * 20 + NUMBASE, 0);

      sprintf(buf, "%02d:%02d:%02d", saved_times[i][0], saved_times[i][1], saved_times[i][2]);
      GrStringDraw(pContext, buf, -1, 25, (i - topView) * 20 + NUMBASE, 0);
      GrLineDrawH(pContext, 0, LCD_WIDTH, (i - topView) * 20 + NUMBASE + 15);
    }

    GrContextFontSet(pContext, &g_sFontGothic18b);
    for(int i = topView; (i < topView + 3) && (i < currentStop); i++)
    {
      if (i == 0)
        continue;

      if (delta_times[i - 1][0] != 0)
      {
        sprintf(buf, "+%02d:%02d:%02d", delta_times[i - 1][0], delta_times[i - 1][1],delta_times[i - 1][2]);
        GrStringDraw(pContext, buf, -1, 70, (i - topView) * 20 + NUMBASE +  3, 0);
      }
      else
      {
        sprintf(buf, "+%02d:%02d", delta_times[i - 1][1],delta_times[i - 1][2]);
        GrStringDraw(pContext, buf, -1, 90, (i - topView) * 20 + NUMBASE + 3, 0);
      }
      
    }
  }

  switch(state)
  {
    case STATE_INIT:
    window_button(pContext, KEY_ENTER, "START");
    break;
    case STATE_RUNNING:
    window_button(pContext, KEY_ENTER, "STOP");
    window_button(pContext, KEY_EXIT,  "STOP ALL");
    break;
    case STATE_STOP:
    window_button(pContext, KEY_UP, "UP");
    window_button(pContext, KEY_DOWN, "DOWN");
    break;
  }
}

static void watch_stop()
{
  state = STATE_STOP;
  rtc_enablechange(0); 
  window_invalid(NULL);
}

uint8_t stopwatch_process(uint8_t event, uint16_t lparam, void* rparam)
{
  switch(event)
  {
    case EVENT_WINDOW_CREATED:
    state = STATE_INIT;
    topView = currentStop = 0;
    times[0] = times[1] = times[2] = 0;
    break;
    case EVENT_WINDOW_PAINT:
    OnDraw((tContext*)rparam);
    break;
    case EVENT_TIME_CHANGED:
    {
      // 32Hz interrupt
      tRectangle rect = {0, 45, LCD_Y_SIZE, 70};
      counter+=2;
      if (counter >= 32)
      {
        times[1]++;
        counter -= 32;
      }
      times[2] = counter * 3;
      if (times[1] == 60)
      {
        times[1] = 0;
        times[0]++;
      }

      if (times[0] == 60)
      {
          // overflow
        watch_stop();
      }
      window_invalid(&rect);
      break;
    }
    case EVENT_KEY_PRESSED:
    {
      if (lparam == KEY_DOWN)
      {
        if (topView < currentStop - 3)
          topView++;
        window_invalid(NULL);
      }
      else if (lparam == KEY_UP)
      {
        if (topView > 0)
          topView--;
        window_invalid(NULL);
      }
      else if (lparam == KEY_ENTER)
      {
        if (state == STATE_STOP || state == STATE_INIT)
        {
          // let's start
          rtc_enablechange(TENMSECOND_CHANGE);
          state = STATE_RUNNING;
          times[0] = times[1] = times[2] = 0;
          topView = currentStop = 0;
          counter = 0;
        }
        else
        {
          if (state != STATE_STOP)
          {
            saved_times[currentStop][0] = times[0];
            saved_times[currentStop][1] = times[1];
            saved_times[currentStop][2] = times[2];

            if (currentStop > 0)
            {
#if 0
              int delta = (saved_times[currentStop][0] * 3600 + saved_times[currentStop][1] * 60 + saved_times[currentStop][2]) -
              (saved_times[currentStop - 1][0] * 3600 + saved_times[currentStop - 1][1] * 60 + saved_times[currentStop - 1][2]);
              delta_times[currentStop - 1][2] = delta % 60;
              delta = delta / 60;
              delta_times[currentStop - 1][1] = delta % 60;
              delta_times[currentStop - 1][0] = delta / 60;
#endif
              delta_times[currentStop - 1][2] = saved_times[currentStop][2] - saved_times[currentStop - 1][2];
              delta_times[currentStop - 1][1] = saved_times[currentStop][1] - saved_times[currentStop - 1][1];
              delta_times[currentStop - 1][0] = saved_times[currentStop][0] - saved_times[currentStop - 1][0];

              if (delta_times[currentStop - 1][2] < 0)
              {
                delta_times[currentStop - 1][1]--;
                delta_times[currentStop - 1][2]+=100;
              }

              if (delta_times[currentStop - 1][1] < 0)
              {
                delta_times[currentStop - 1][0]--;
                delta_times[currentStop - 1][1]+=60;
              }
            }

            currentStop++;
            if ((topView + 4 == currentStop) && (currentStop > 3))
            {
              topView++;
            }
          }

          if (currentStop >= MAX_STOP)
          {
            watch_stop();
          }
        }

        window_invalid(NULL);
      }
      break;
    }
    case EVENT_EXIT_PRESSED:
    if (state == STATE_STOP)
      window_close();
    else
      watch_stop();
    break;
    case EVENT_WINDOW_CLOSING:
    {
      rtc_enablechange(0);
    }
    default:
    return 0;
  }

  return 1;
}
