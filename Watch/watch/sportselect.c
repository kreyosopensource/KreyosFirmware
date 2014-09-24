#include "contiki.h"
#include "window.h"
#include "memlcd.h"

static uint8_t selection;
static const struct 
{
  char icon;
  const char* text;
}selections[] = 
{
  {'b', "Running"},
  {'a', "Cycling"},
};

static void onDraw(tContext *pContext)
{
  int width;
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &fullscreen_clip);

  GrContextForegroundSet(pContext, ClrWhite);

  for(int i = 0; i < 2; i++)
  {
  	char buf = selections[i].icon;

    GrContextForegroundSet(pContext, ClrWhite);
  	if (i == selection)
  	{
  		tRectangle rect = {14, 10 + i * 75, LCD_WIDTH - 14, 77 + i * 75};
		  GrRectFillRound(pContext, &rect, 3);

  		GrContextForegroundSet(pContext, ClrBlack);
  	}

    GrContextFontSet(pContext, (const tFont*)&g_sFontExIcon48);
    width = GrStringWidthGet(pContext, &buf, 1);
    GrStringDraw(pContext, &buf, 1, (LCD_WIDTH - width ) /2, 10 + i * 75, 0);

    GrContextFontSet(pContext, &g_sFontGothic18b);
    width = GrStringWidthGet(pContext, selections[i].text, -1);
    GrStringDraw(pContext, selections[i].text, -1, (LCD_WIDTH - width ) /2, 55 + i * 75, 0);
  }
}

// select sport type
uint8_t sporttype_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev){
  case EVENT_WINDOW_CREATED:
    return 0x80;
  case EVENT_WINDOW_PAINT:
  	onDraw((tContext*)rparam);
  	break;
  case EVENT_KEY_PRESSED:
  	if (lparam == KEY_UP && selection > 0)
  		selection--;
  	if (lparam == KEY_DOWN && selection < 1)
  		selection++;
  	if (lparam == KEY_ENTER)
  	{
      window_close();
  		window_open(&sportwait_process, (void*)selection);
  		return 1;
  	}
  	window_invalid(NULL);
  	break;
  case EVENT_NOTIFY_RESULT:
  	window_close();
  	break;
  default:
  	return 0;
  }

  return 1;
}
