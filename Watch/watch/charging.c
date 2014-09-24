#include "contiki.h"
#include "battery.h"
#include "window.h"
#include "icons.h"
#include "memlcd.h"

#include "grlib/grlib.h"

uint8_t state = 0;
uint8_t charging_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  	case EVENT_WINDOW_ACTIVE:
  	{
  		state = 0;
  		// Fallthrough
	}

	case PROCESS_EVENT_TIMER:
	{
		// check the battery status
		switch(battery_state())
		{
			case BATTERY_STATE_DISCHARGING:
			if (battery_level(BATTERY_STATE_DISCHARGING) == 0)
			{
				state = 6;
				window_invalid(NULL);
			}
			else
			{
				window_close();
				return 1;
			}
			break;
			case BATTERY_STATE_CHARGING:
			state++;
			if (state >= 4)
				state = 0;
			window_invalid(NULL);
			break;
			case BATTERY_STATE_FULL:
			state = 5;
			window_invalid(NULL);
			break;
		}
		window_timer(CLOCK_SECOND);
		break;
	}

    case EVENT_WINDOW_PAINT:
    {
      tContext *pContext = (tContext*)rparam;

	  // clear the region
	  GrContextForegroundSet(pContext, ClrBlack);
	  GrRectFill(pContext, &client_clip);

	  GrContextForegroundSet(pContext, ClrWhite);

	  // draw the icon
	  GrContextFontSet(pContext, (const tFont*)&g_sFontExIcon48);

	  if (state == 5)
	  {
	  	char icon = ICON_LARGE_BATTERY_LEVEL4;
	  	GrStringDrawCentered(pContext, &icon, 1, LCD_WIDTH/2, 37, 0);

	  	GrContextFontSet(pContext, (tFont*)&g_sFontGothic24b);
	  	GrStringDrawWrap(pContext, "Battery is fully charged.", 10, 90, LCD_WIDTH - 20, ALIGN_CENTER);
	  }
	  else if (state == 6)
	  {
	  	char icon = ICON_LARGE_LOWBATTERY;
	  	GrStringDrawCentered(pContext, &icon, 1, LCD_WIDTH/2, 37, 0);

	  	GrContextFontSet(pContext, (tFont*)&g_sFontGothic24b);
	  	GrStringDrawWrap(pContext, "LOW BATTERY\nConnect charger now.", 10, 90, LCD_WIDTH - 20, ALIGN_CENTER);	  	
	  }
	  else
	  {
	  	char icon = ICON_LARGE_BATTERY_LEVEL1 + state;
	  	GrStringDrawCentered(pContext, &icon, 1, LCD_WIDTH/2, 37, 0);

		GrContextFontSet(pContext, (tFont*)&g_sFontGothic24b);
	  	GrStringDrawWrap(pContext, "Battery is charging.", 10, 90, LCD_WIDTH - 20, ALIGN_CENTER);
	  }
      break;
	}
	
	default:
		return 0;

    }

    return 1;
}
