#include "contiki.h"
#include "window.h"
#include "icons.h"
#include "grlib/grlib.h"
#include "memlcd.h"

#include "bluetooth.h"

static enum {BT_ON, BT_OFF, BT_INITIALING, BT_W4PAIR, BT_W4PAIR2} state;
PROCESS_NAME(bluetooth_process);

void draw_screen(tContext *pContext)
{
  char icon;
  int offset = 0;
  const char* str;
  // clear the region
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &client_clip);

  // display text
  if (state == BT_ON)
  {
    icon = ICON_LARGE_BT;
    str = "Bluetooth is ON";
    window_button(pContext, KEY_UP, "OFF");
    window_button(pContext, KEY_DOWN, "Exit");
  }
  else if (state == BT_OFF)
  {
    icon = ICON_LARGE_NOBT;
    window_button(pContext, KEY_DOWN, "ON");
    str = "Bluetooth is OFF";
  }
  else if (state == BT_W4PAIR)
  {
    icon = ICON_LARGE_WAIT1;
    state = BT_W4PAIR2;
    str = "Please wait...";
    window_button(pContext, KEY_UP, "OFF");
  }
  else if (state == BT_W4PAIR2)
  {
    icon = ICON_LARGE_WAIT2;
    state = BT_W4PAIR;
    offset = 15;
    str = "Please wait...";
    window_button(pContext, KEY_UP, "OFF");
  }
  else if (state == BT_INITIALING)
  {
    icon = ICON_LARGE_NOBT;
    str = "Initializing Bluetooth";
  }
  else
  {
    return;
  }

  GrContextForegroundSet(pContext, ClrWhite);
  GrContextBackgroundSet(pContext, ClrBlack);

  GrContextFontSet(pContext, (const tFont*)&g_sFontExIcon48);
  GrStringDraw(pContext, &icon, 1, 50 + offset, 50, 0);

  GrContextFontSet(pContext, &g_sFontGothic18b);
  GrStringDrawCentered(pContext, str, -1, LCD_WIDTH/2, 105, 0);
}

uint8_t btconfig_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  case EVENT_WINDOW_CREATED:
    {
      // check the btstack status
      if (bluetooth_running())
      {
        state = BT_W4PAIR;

        // if btstack is on, make it discoverable
        bluetooth_discoverable(1);
      }
      else
      {
        state = BT_OFF;
      }

      return 1;
    }
  case EVENT_WINDOW_ACTIVE:
  {
    window_timer(CLOCK_SECOND);
    break;
  }
  case PROCESS_EVENT_TIMER:
  {
    window_timer(CLOCK_SECOND);
    window_invalid(NULL);
    break;
  }
  case EVENT_WINDOW_PAINT:
    {
      draw_screen((tContext*)rparam);
      return 1;
    }
  case EVENT_BT_STATUS:
    {
      if ((lparam == BT_INITIALIZED) && (state == BT_OFF || state == BT_INITIALING))
      {
        bluetooth_discoverable(1);
        state = BT_W4PAIR;
      }
      else if ((lparam == BT_SHUTDOWN) && state == BT_ON)
      {
        state = BT_OFF;
      }
      else if (lparam == BT_CONNECTED)
      {
        state = BT_ON;
      }

      window_invalid(NULL);
      return 1;
    }
  case EVENT_KEY_PRESSED:
    {
      if (lparam == KEY_DOWN)
      {
        if (state == BT_ON)
        {
          window_close();
        }
        else if (state == BT_OFF)
        {
          state = BT_INITIALING;
          bluetooth_start();
        }
        window_invalid(NULL);
        return 1;
      }
      else if (lparam == KEY_UP)
      {
        if (state == BT_ON || state == BT_W4PAIR2 || state == BT_W4PAIR)
        {
          bluetooth_shutdown();
          state = BT_OFF;          
        }
      }
      break;
    }
  case EVENT_WINDOW_CLOSING:
    {
      bluetooth_discoverable(0);
      window_timer(0);
      break;
    }
  }

  return 0;
}
