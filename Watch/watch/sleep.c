#include "contiki.h"
#include "grlib/grlib.h"
#include "memlcd.h"
#include "window.h"
#include "pedometer/sleepalgo.h"

#include <stdlib.h>
#include <stdio.h>

static uint8_t *ptr;
static uint16_t offset;

#define LINEMARGIN 25
static void drawItem(tContext *pContext, uint8_t n, char icon, const char* text, const char* value)
{
  if (icon)
  {
    GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);
    GrStringDraw(pContext, &icon, 1, 3, 30 + n * LINEMARGIN, 0);
  }

  // draw text
  GrContextFontSet(pContext, &g_sFontGothic18b);
  GrStringDraw(pContext, text, -1, 10, 30 + n * LINEMARGIN, 0);

  uint8_t width = GrStringWidthGet(pContext, value, -1);
  GrStringDraw(pContext, value, -1, LCD_WIDTH - width - 8, 30 + n * LINEMARGIN, 0);
}

static void formattime(char* buf, int minutes)
{
	uint16_t hours = minutes / 60;
	minutes = minutes % 60;

	sprintf(buf, "%d:%02d", hours, minutes);
}


extern void mpu_switchmode(int d);
uint8_t test_sleep(uint8_t ev, uint16_t lparam, void* rparam)
{
	switch(ev)
	{
		case EVENT_WINDOW_CREATED:
		ptr = (uint8_t*)malloc(128);
		if (ptr)
		{
			*ptr = 0xff;
			sleepalgo_init(ptr, 128);
			mpu_switchmode(2);
			window_timer(CLOCK_SECOND * 60);
		}
		break;
		case EVENT_WINDOW_ACTIVE:
		window_timer(CLOCK_SECOND * 60);
		break;

		case EVENT_WINDOW_PAINT:
		{
		  tContext *pContext = (tContext*)rparam;
		  GrContextForegroundSet(pContext, ClrBlack);
		  GrRectFill(pContext, &client_clip);

		  GrContextForegroundSet(pContext, ClrWhite);
      	  unsigned int available_minutes, lost_minutes;
      	  slp_get_availabledatainfo(&available_minutes, &lost_minutes);
          GrContextFontSet(pContext, (tFont*)&g_sFontGothic18);
		  char buf[20];		  
		  formattime(buf, slp_getfallasleep_time());
		  drawItem(pContext, 0, 0, "Time to Sleep", buf);

		  formattime(buf, slp_get_classify_time(SLEEP_LEVEL0));
		  drawItem(pContext, 1, 0, "Awake Time", buf);

		  formattime(buf, slp_get_classify_time(SLEEP_LEVEL1));
		  drawItem(pContext, 2, 0, "Sleep", buf);

		  formattime(buf, slp_get_classify_time(SLEEP_LEVEL2));
		  drawItem(pContext, 3, 0, "Deep Sleep", buf);

		  sprintf(buf, "%d", slp_get_wakeup_times());
		  drawItem(pContext, 4, 0, "Wake Times", buf);

		  // draw diagram
		  for(int i = offset; i < ((available_minutes + 3) & 0xFC) && i < offset + LCD_WIDTH; i+=4)
		  {
			uint8_t data = ptr[i];
		  	for (int j = i; j < i + 4 && j < available_minutes; j++)
		  	{
		  		int length;
		  		switch(data & 0xC0)
		  		{
		  			case 0:
		  				length = 1;
		  				break;
		  			case 0x40:
		  				length = 6;
		  				break;
		  			case 0x80:
		  				length = 15;
		  				break;
		  			default:
		  				continue;
		  		}

		  		GrLineDrawV(pContext, j - offset, LCD_Y_SIZE-length, LCD_Y_SIZE);
		  		data <<= 2;
		  	}
		  }

 		  break;
 		}

 		case EVENT_KEY_PRESSED:
 		{
 			if (lparam == KEY_UP)
 			{
 				if (offset > 24)
 					offset -= 24;
 				else
 					offset = 0;
 			}
 			else if (lparam == KEY_DOWN)
 			{
 				unsigned int available_minutes, lost_minutes;
	      	  	slp_get_availabledatainfo(&available_minutes, &lost_minutes);

 				offset += 24;
 				if (offset + LCD_WIDTH < available_minutes)
 					offset -= 24;
 			}
 			window_invalid(NULL);
 			break;
 		}

 		case PROCESS_EVENT_TIMER:
 		{
 			window_invalid(NULL);
 			window_timer(CLOCK_SECOND * 60);
 			break;
 		}

 		case EVENT_WINDOW_CLOSING:
	 		printf("stop_slp_monitor()\n");
	 		mpu_switchmode(0);
	 		slp_stop_monitor();
	 		window_timer(0);
	 		if (ptr)
	 			free(ptr);
	 		ptr = NULL;
	 		break;

 		default:
 		return 0;
	}

	return 1;
}
