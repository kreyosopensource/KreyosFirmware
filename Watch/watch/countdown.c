
#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "cordic.h"
#include "backlight.h"

#include "memory.h"

enum _state{
  STATE_CONFIG_HOUR,
  STATE_CONFIG_MINUTE,
  STATE_CONFIG_SECOND,

  STATE_CONFIG_READY, // the order above is assumed in the logic, don't change
  STATE_CONFIG_PAUSE,
  STATE_CONFIG_DONE,
  STATE_RUNNING
};

#define state d.countdown.state
#define times d.countdown.times
#define totaltime d.countdown.totaltime
#define lefttime d.countdown.lefttime

#define CENTER_X 75
#define CENTER_Y 95

#define INNER_R 16
#define OUTER_R 32

/* static const tRectangle progress_range = 
{
  CENTER_X - OUTER_R, CENTER_Y - OUTER_R,
  CENTER_X + OUTER_R, CENTER_Y + OUTER_R,
};*/

static void OnDrawProgress(tContext *pContext)
{
  uint16_t range = 360 - lefttime * 360 / totaltime;
  uint8_t ix, iy, ex, ey;
  uint8_t ixp, iyp, exp, eyp;

  int sin_val, cos_val;

  for(int angle = 0; angle < range; angle += 8)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);

    ixp = CENTER_X + ((16 * (sin_val >> 8)) >> 7);
    iyp = CENTER_Y - ((16 * (cos_val >> 8)) >> 7);

    exp = CENTER_X + ((29 * (sin_val >> 8)) >> 7);
    eyp = CENTER_Y - ((29 * (cos_val >> 8)) >> 7);

    if (angle + 8 > range)
      angle = range;
    else
      angle += 8;
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    ix = CENTER_X + ((16 * (sin_val >> 8)) >> 7);
    iy = CENTER_Y - ((16 * (cos_val >> 8)) >> 7);

    ex = CENTER_X + ((29 * (sin_val >> 8)) >> 7);
    ey = CENTER_Y - ((29 * (cos_val >> 8)) >> 7);

    GrTriagleFill(pContext, ixp, iyp, exp, eyp, ix, iy);
    GrTriagleFill(pContext, ix, iy, ex, ey, exp, eyp);
  }
}

static void OnDraw(tContext *pContext)
{
  GrContextFontSet(pContext, &g_sFontGothic28b);

  // clear the region
  GrContextForegroundSet(pContext, ClrBlack);
  GrContextBackgroundSet(pContext, ClrWhite);
  GrRectFill(pContext, &client_clip);

  GrContextForegroundSet(pContext, ClrWhite);
  // draw the button text
  switch(state)
  {
  case STATE_CONFIG_SECOND:
  case STATE_CONFIG_MINUTE:
  case STATE_CONFIG_HOUR:
    {
      window_drawtime(pContext, 65, times, 1 << state);
      window_button(pContext, KEY_ENTER, "OK");
      break;
    }
  case STATE_CONFIG_READY:
    {
      window_drawtime(pContext, 27, times, 1 << state);

      window_button(pContext, KEY_DOWN, "RESET");
      window_button(pContext, KEY_ENTER, "START");

      break;
    }
  case STATE_CONFIG_PAUSE:
    {
      window_drawtime(pContext, 27, times, 1 << state);

      window_button(pContext, KEY_DOWN, "RESET");
      window_button(pContext, KEY_ENTER, "Continue");

      // display progress bar
      if (totaltime != lefttime)
        OnDrawProgress(pContext);

      break;
    }
  case STATE_CONFIG_DONE:
    {
      window_drawtime(pContext, 27, times, 1 << state);

      window_button(pContext, KEY_DOWN, "RESET");

      // display progress bar
      if (totaltime != lefttime)
      {
        GrContextForegroundSet(pContext, ClrWhite);
        OnDrawProgress(pContext);
      }

      break;      
    }
  case STATE_RUNNING:
    {
      window_drawtime(pContext, 27, times, 1 << state);
      window_button(pContext, KEY_DOWN, "PAUSE");
      window_button(pContext, KEY_ENTER, "STOP");

      // display progress bar
      GrContextForegroundSet(pContext, ClrWhite);
      OnDrawProgress(pContext);

      break;
    }
  }
}

static int process_event(uint8_t ev, uint16_t data)
{
  switch(state)
  {
  case STATE_CONFIG_SECOND:
  case STATE_CONFIG_MINUTE:
  case STATE_CONFIG_HOUR:
    {
      // handle up, down
      if (ev != EVENT_KEY_PRESSED)
        return 0;
      switch(data)
      {
      case KEY_UP:
        times[state]++;
        // limit to one day
        if ((times[state] >= 60) || (times[state] >= 24 && state == STATE_CONFIG_HOUR))
        {
          times[state] = 0;
        }
        break;
      case KEY_DOWN:
        times[state]--;
        if (times[state] == 0xff)
        {
          if (state == STATE_CONFIG_HOUR)
            times[state] = 23;
          else
            times[state] = 59;
        }
        break;
      case KEY_ENTER:
        if (state == STATE_CONFIG_HOUR)
        {
          if (times[0] != 0  || times[1] != 0  || times[2] != 0)
            state = STATE_CONFIG_READY;
          else
            state = STATE_CONFIG_SECOND;
        }
        else
          state--;
        break;
      }
      window_invalid(NULL);

      return 1;
    }
    case STATE_CONFIG_DONE:
    {

      if (ev != EVENT_KEY_PRESSED)
        return 0;

      if ((uint8_t)data == KEY_DOWN)
      {
        state = STATE_CONFIG_SECOND;
        times[0] = times[1] = times[2] = 0;
      }
      window_invalid(NULL);

      return 1;
    }
  case STATE_CONFIG_PAUSE:
  case STATE_CONFIG_READY:
    {
      if (ev != EVENT_KEY_PRESSED)
        return 0;

      if ((uint8_t)data == KEY_DOWN)
      {
        state = STATE_CONFIG_SECOND;
        times[0] = times[1] = times[2] = 0;
      }
      else if (data == KEY_ENTER)
      {
        if (state == STATE_CONFIG_READY)
        {
          lefttime = totaltime = times[STATE_CONFIG_SECOND] + times[STATE_CONFIG_MINUTE] * 60
              + times[STATE_CONFIG_HOUR] * 3600;
        }
        // setup timer every second
        window_timer(CLOCK_SECOND);

        state = STATE_RUNNING;
      }
      window_invalid(NULL);

      return 1;
    }
  case STATE_RUNNING:
    {
      if (ev == EVENT_KEY_PRESSED)
      {
        if (data == KEY_DOWN)
        {
          // pause
          window_timer(0);
          state = STATE_CONFIG_PAUSE;
        }
        else if (data == KEY_ENTER)
        {
          // stop
          totaltime = lefttime;
          window_timer(0);
          state = STATE_CONFIG_READY;
        }
      }
      else if (ev == PROCESS_EVENT_TIMER)
      {
        lefttime--;
        times[STATE_CONFIG_SECOND] = lefttime % 60;
        times[STATE_CONFIG_MINUTE] = (lefttime/60) % 60;
        times[STATE_CONFIG_HOUR] = (lefttime/3600) % 60;
        if (lefttime == 0)
        {
          // trigger notification
          motor_on(100, CLOCK_SECOND * 2);
          state = STATE_CONFIG_DONE;
        }
        else
        {
          window_timer(CLOCK_SECOND);
        }
      }
      else
        return 0;

      window_invalid(NULL);
      return 1;
    }
  }

  return 0;
}

uint8_t countdown_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  if (ev == EVENT_WINDOW_CREATED)
  {
    state = STATE_CONFIG_SECOND;
    times[0] = times[1] = times[2] = 0;
  }
  else if (ev == EVENT_WINDOW_CLOSING)
  {
    // remove timer
    window_timer(0);
  }
  else if (ev == EVENT_WINDOW_PAINT)
  {
     OnDraw((tContext*)rparam);
  }
  else
  {
    return process_event(ev, lparam);
  }

  return 1;
}
