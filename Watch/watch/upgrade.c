#include "stdio.h"
#include "contiki.h"
#include "window.h"

static uint8_t progress;
extern int CheckUpgrade(void);
extern void system_reset();

#define PROGRESS_FINISH 100
#define PROGRESS_TIMEOUT 254

static void onDraw(tContext *pContext)
{
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &client_clip);
  GrContextForegroundSet(pContext, ClrWhite);
  
  GrContextFontSet(pContext, &g_sFontGothic18b);
  GrStringDrawCentered(pContext, "Firmware Upgrade", -1, 72, 60, 0);
  if (progress == PROGRESS_FINISH)
  	GrStringDrawCentered(pContext, "Done", -1, 72, 96, 0);
  else if (progress == PROGRESS_TIMEOUT)
  	GrStringDrawCentered(pContext, "Failed due to timeout", -1, 72, 96, 0);
  else
  	GrStringDrawCentered(pContext, "Don't do any operation", -1, 72, 96, 0);

  if (progress == PROGRESS_FINISH)
  	window_button(pContext, KEY_ENTER, "Reboot");
  else if (progress == PROGRESS_TIMEOUT)
  	window_button(pContext, KEY_ENTER, "Exit");
  else
	window_progress(pContext, 110, progress);
}

extern uint8_t upgrade_process(uint8_t ev, uint16_t lparam, void* rparam)
{
	switch(ev)
	{
		case EVENT_WINDOW_CREATED:
			window_timer(CLOCK_SECOND * 30);
			break;
		case EVENT_FIRMWARE_UPGRADE:
			if (rparam == (void*)PROGRESS_TIMEOUT)
				break;

			if (rparam == (void*)-1)
			{
				progress = PROGRESS_FINISH;
				window_timer(0);
			}
			else
			{
				progress = (long)rparam * 100/(230UL*1024);
				window_timer(CLOCK_SECOND * 30);
			}
			window_invalid(NULL);
			break;
		case EVENT_WINDOW_PAINT:
			onDraw((tContext*)rparam);
		    break;
		case EVENT_EXIT_PRESSED:
			if (progress != PROGRESS_FINISH || progress != PROGRESS_TIMEOUT)
				return 1;
			break;
		case PROCESS_EVENT_TIMER:
			if (progress != PROGRESS_FINISH)
			{
				progress = PROGRESS_TIMEOUT;
				window_invalid(NULL);
			}
			break;
		case EVENT_KEY_PRESSED:
		 	if (lparam == KEY_ENTER)
		 	{
		 		if (progress == PROGRESS_FINISH)
		 		{
		 			int ret = CheckUpgrade();
		 			printf("CheckUpgrade() %d\n", ret);
		 			if (ret == 0xff)
		 				system_reset();
		 			else
		 				window_close();
		 		}
		 		else if (progress == PROGRESS_TIMEOUT)
		 		{
		 			window_close();
		 		}
		 	}
		 	break;
	}
	return 1;
}
