#include "contiki.h"
#include "grlib/grlib.h"
#include "window.h"
#include "bluetooth.h"
#include "memlcd.h"
#include "icons.h"
#include <stdio.h>
#include "system.h"

static uint8_t state;

static void OnDraw(tContext *pContext)
{
  GrContextForegroundSet(pContext, ClrBlack);
  static const tRectangle rect = {0, 0, LCD_WIDTH, LCD_Y_SIZE};
  GrRectFill(pContext, &rect);

  // draw the log
  GrContextForegroundSet(pContext, ClrWhite);
  GrContextBackgroundSet(pContext, ClrBlack);

  // draw welcome
  GrContextFontSet(pContext, (tFont*)&g_sFontExIcon48);
  char icon = ICON_LARGET_WATCH;
  GrStringDraw(pContext, &icon, 1, 12, 16, 0);
  icon = ICON_LARGET_PHONE;
  GrStringDraw(pContext, &icon, 1, 99, 16, 0);

  icon = ICON_BT; // small bt icon
  GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);
  GrStringDraw(pContext, &icon, 1, 64, 35, 0);

  tRectangle rect2 = {48, 40, 48 + 3, 40 + 3}; 
  for(int i = 0; i < 3; i++)
  {
  	GrRectFill(pContext, &rect2);
  	rect2.sXMin = rect2.sXMax + 2;
  	rect2.sXMax = rect2.sXMin + 3;
  }

  rect2.sXMin = 74;
  rect2.sXMax = 76;
  for(int i = 0; i < 3; i++)
  {
  	GrRectFill(pContext, &rect2);
  	rect2.sXMin = rect2.sXMax + 2;
  	rect2.sXMax = rect2.sXMin + 3;
  }

  GrContextFontSet(pContext, (tFont*)&g_sFontGothic28b);
  GrStringDrawCentered(pContext, "Install the", -1, LCD_WIDTH/2, 78, 0);
  GrStringDrawCentered(pContext, "Kreyos App", -1, LCD_WIDTH/2, 103, 0);

  GrContextFontSet(pContext, (tFont*)&g_sFontGothic18);
  GrStringDrawCentered(pContext, "kreyos.com/setup", -1, LCD_WIDTH/2, 123, 0);

  static const tRectangle rect3 = {0, 140, LCD_WIDTH, LCD_Y_SIZE};
  GrRectFill(pContext, &rect3);

  GrContextForegroundSet(pContext, ClrBlack);
  GrContextFontSet(pContext, (tFont*)&g_sFontGothic18b);

  if (bluetooth_running())
  {
    char buf[20];
    const char* btaddr = (const char*)system_getserial();
    if (state == 0)
      sprintf(buf, "Meteor %02X%02X", btaddr[4], btaddr[5]);
    else
      sprintf(buf, "Meteor %02X%02X [%d]", btaddr[4], btaddr[5], state);
    GrStringDrawCentered(pContext, buf, -1, LCD_WIDTH/2, 153, 0);
  }
}

uint8_t welcome_process(uint8_t ev, uint16_t lparam, void* rparam)
{
	//static const tRectangle rect = {0, 60, LCD_WIDTH, 160};
	switch(ev)
	{
		case EVENT_WINDOW_CREATED:
		      // check the btstack status
		    if (bluetooth_running())
		    {
		      // if btstack is on, make it discoverable
		      bluetooth_discoverable(1);
		    }
      state = 0;
			return 0x80;

		case EVENT_WINDOW_PAINT:
			OnDraw((tContext*)rparam);
			break;

		case EVENT_BT_STATUS:
    	{
    		if (lparam == BT_INITIALIZED)
    		{
      		bluetooth_discoverable(1);
	      }
        window_invalid(NULL);
		    break;
		  }

    case EVENT_EXIT_PRESSED:
    break;

		case EVENT_KEY_PRESSED:
    {
      switch(lparam)
      {
        case KEY_ENTER:
        if (state == 0)
        {
          state++;
        }
        else
        {
          state = 0;
        }
        break;

        case KEY_UP:
        if (state == 1 || state == 3)
        {
          state++;
        }
        else
        {
          state = 0;
        }
        break;

        case KEY_DOWN:
        if (state == 2)
        {
          state++;
        }
        else if (state == 4)
        {
          system_unlock();
        }
        else
        {
          state = 0;
        }
        break;
      }
      window_invalid(NULL);
			break;
    }

    default:
      return 0;
	}

	return 1;

}
