#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "memlcd.h"
#include "backlight.h"
#include "status.h"
#include "rtc.h"
#include "btstack/ble/att_client.h"
#include "btstack/ble/ancs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* message_title;
static const char* message_subtitle;
static const char* message;
static const char* message_date;
static char message_icon;
static uint8_t message_buttons;
static uint8_t message_result;

static uint32_t lastmessageid = -1;

#define BORDER 5

static const tRectangle contentrect = 
{0, 12, LCD_WIDTH, LCD_Y_SIZE};

static enum
{
  STATE_ACTIVE = 0x01,
  STATE_MORE = 0x02,
  STATE_PENDING = 0x04
}state;

static int skip = 0;
static uint8_t notify_process(uint8_t ev, uint16_t lparam, void* rparam);

#define EMPTY 0xfe
#define SPECIAL 0xff

#define MAX_NOTIFY 5

static uint8_t  num_uids = 0;
static uint8_t  selectidx = 0;
static uint32_t uids[MAX_NOTIFY];
static uint32_t attributes[MAX_NOTIFY];

static void onDrawTitleBar(tContext *pContext)
{
  // draw the title bar of circles
  const tRectangle rect = {0, 0, LCD_WIDTH, 12};

  GrContextForegroundSet(pContext, ClrBlack);
  GrContextClipRegionSet(pContext, &rect);

  GrRectFill(pContext, &rect);

  GrContextForegroundSet(pContext, ClrWhite);

  long startx = LCD_WIDTH/2 - num_uids * 4;

  for(int i = 0; i < num_uids; i++)
  {
    if (i == selectidx)
      GrCircleFill(pContext, startx + i * 10, 4, 3);
    else
      GrCircleDraw(pContext, startx + i * 10, 4, 3);
  }
}

static const tFont *get_titlefont()
{
  switch(window_readconfig()->font_config)
  {
    case 1:
      return (const tFont*)&g_sFontGothic28b;
    case 2:
      return (const tFont*)&g_sFontUnicode;
      break;
    default:
      return (const tFont*)&g_sFontGothic24b;
  }
}

static const tFont *get_contentfont()
{
  switch(window_readconfig()->font_config)
  {
    case 1:
      return (const tFont*)&g_sFontGothic24b;
    case 2:
      return (const tFont*)&g_sFontUnicode;
    case 0:
    default:
      return (const tFont*)&g_sFontGothic18b;
  }
  
}


static const char* parse_date(char* date)
{
  // date in the format of yyyyMMdd'T'HHmmSS
  //                       01234567 8 9abcde
  uint8_t event_second, event_minute, event_hour;
  uint8_t event_day, event_month;
  uint16_t event_year;

  event_second = atoi(&date[0x0d]); date[0x0d] = '\0';
  event_minute = atoi(&date[0x0b]); date[0x0b] = '\0';
  event_hour   = atoi(&date[0x09]); date[0x08] = '\0';

  event_day    = atoi(&date[0x06]); date[0x06] = '\0';
  event_month  = atoi(&date[0x04]); date[0x04] = '\0';
  event_year   = atoi(&date[0x00]);

  uint32_t event_timestamp = calc_timestamp(event_year - 2000, event_month, event_day, event_hour, event_minute, event_second);
  uint32_t now_timestamp = rtc_readtime32();

  if (event_timestamp > now_timestamp)
  {
    // event happen later than now, this should not happen, adjust rtc
    rtc_settime(event_hour, event_minute, event_second);
    rtc_setdate(event_year, event_month, event_day);

    now_timestamp = event_timestamp;
  }
  
  return toEnglishPeriod(now_timestamp - event_timestamp, date);
}

static void onDraw(tContext *pContext)
{
  // draw circles
  onDrawTitleBar(pContext);

  GrContextClipRegionSet(pContext, &contentrect);
  GrContextForegroundSet(pContext, ClrWhite);
  GrRectFill(pContext, &fullscreen_clip);

  GrContextForegroundSet(pContext, ClrBlack);

  long starty = 12 - skip;

  if (message_icon && message_date)
  {
    // draw the title bar
    // draw icon
    if (message_icon)
    {
      GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);    
      GrStringDraw(pContext, &message_icon, 1, 8, starty, 0);
    }

    if (message_date && message_date[0] != '\0')
    {
      char buffer[20];

      strcpy(buffer, message_date);
      GrContextFontSet(pContext, (tFont*)&g_sFontGothic14);
      const char* text = parse_date(buffer);
      int16_t width = GrStringWidthGet(pContext, text, -1);
      GrStringDraw(pContext, text, -1, LCD_WIDTH - 10 - width, starty, 0);
    }

    starty += 16;
  }

  const tFont *titleFont;
  const tFont *contentFont;

  titleFont = get_titlefont();
  contentFont = get_contentfont();

  GrContextFontSet(pContext, titleFont);

  // draw title
  if (message_title && (*message_title != '\0'))
  {
    starty = GrStringDrawWrap(pContext, message_title, 1, starty, LCD_WIDTH - 1, 0);
  }

  if (message_subtitle && (*message_subtitle != '\0'))
    starty = GrStringDrawWrap(pContext, message_subtitle, 1, starty, LCD_WIDTH - 1, 0);
    
  GrContextFontSet(pContext, contentFont);
  //draw message
  if (message && *message != '\0')
  {
    if (GrStringDrawWrap(pContext, message, 1, starty, LCD_WIDTH - 1,  0) == -1)
    {
      state |= STATE_MORE;
      for(int i = 0; i < 6; i++)
      {
          GrLineDrawH(pContext, 130 - i, 130 + i,  160 - i);
      }
    }
    else
    {
      state &= ~STATE_MORE;
    }
  }
}

static void push_uid(uint32_t id, uint32_t attribute)
{
  if (num_uids <= MAX_NOTIFY - 1)
    num_uids++;

  for(int i = num_uids - 1; i >= 0; i--)
  {
    uids[i] = uids[i - 1];
    attributes[i] = attributes[i - 1];
  }

  uids[0] = id;
  attributes[0] = attribute;
}

void window_notify(const char* title, const char* msg, uint8_t buttons, char icon)
{
  message_title = title;
  message_subtitle = NULL;
  message = msg;
  message_buttons = buttons;
  message_icon = icon;
  skip = 0;

  push_uid(SPECIAL, 0);

  selectidx = 0;

  motor_on(50, CLOCK_SECOND);
  backlight_on(window_readconfig()->light_level, CLOCK_SECOND * 3);

  if (state & STATE_ACTIVE)
    window_invalid(NULL);
  else 
    window_open(notify_process, NULL);
}

void fetch_content()
{
  uint32_t uid;
  uint32_t attribute;
  message_title = NULL;
  message = NULL;
  message_subtitle = NULL;

  uid = uids[selectidx];
  attribute = attributes[selectidx];

  att_fetch_next(uid, attribute);
}

void window_notify_ancs_init()
{
  lastmessageid = -1;
}

void window_notify_ancs(uint8_t command, uint32_t uid, uint8_t flag, uint8_t category)
{
  if (command == 0) // add
  {
    if (lastmessageid != -1 && lastmessageid >= uid)
    {
      return;
    }

    message_title = NULL;
    message = NULL;
    push_uid(uid, (flag << 8) | category);
    selectidx = 0;
    if (category == CategoryIDIncomingCall)
      motor_on(50, CLOCK_SECOND * 60);
    else
      motor_on(50, CLOCK_SECOND);
    backlight_on(window_readconfig()->light_level, CLOCK_SECOND * 3);

    lastmessageid = uid;
    if (state & STATE_ACTIVE)
      window_invalid(NULL);
    else 
      window_open(notify_process, NULL);

    fetch_content();    
  }
  else if (command == 1)
  {
    if (state & STATE_ACTIVE)
    {
      // check if the current 
      if (uids[selectidx] == uid)
      {
        fetch_content();
      }
      window_invalid(NULL);
      motor_on(50, CLOCK_SECOND);
    }
  }
  else if (command == 2) // remove
  {
    if (!(state & STATE_ACTIVE))
      return;

    uint8_t refresh = 0;
    if (uids[selectidx] == uid)
    {
      refresh = 1;
    }

    // find the item
    int i;
    for(i = 0; i < num_uids; i++)
    {
      if (uids[i] == uid)
        break;
    }

    if (i == num_uids)
      return;

    for (int j = i ; j < num_uids; j++)
    {
      uids[j] = uids[j+1];
      attributes[j] = attributes[j+1];
    }
    num_uids--;

    if (num_uids == 0)
      window_close();
    else if (refresh)
      fetch_content();
  }
}

void window_notify_content(const char* title, const char* subtitle, const char* msg, const char* date, uint8_t buttons, char icon)
{
  uint16_t attr = attributes[selectidx];
  printf("Find phone number %d from %d", attr, selectidx);
  if ((attr & 0xFF) == CategoryIDIncomingCall)
  {
    
    process_post(ui_process, EVENT_BT_CLIP, (void*)title);
  }

  message_title = title;
  message_subtitle = subtitle;
  message = msg;
  message_date = date;
  message_buttons = buttons;
  message_icon = icon;

  skip = 0;

  window_invalid(NULL);
}

// notify window process
static uint8_t notify_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  case EVENT_WINDOW_CREATED:
  {
    state |= STATE_ACTIVE;
    add_watch_status(WS_NOTIFY);
    return 0x80;
  }
  case EVENT_WINDOW_PAINT:
    {
      onDraw((tContext*)rparam);
      break;
    }
  case EVENT_WINDOW_CLOSING:
    state &= ~STATE_ACTIVE;
    process_post(ui_process, EVENT_NOTIFY_RESULT, (void*)message_result);
    motor_on(0, 0);
    del_watch_status(WS_NOTIFY);
    selectidx = 0;
    num_uids = 0;
    return 0;
    break;
  case EVENT_KEY_PRESSED:
    motor_on(0, 0);

    if (message == NULL)
      break;
    
    if (lparam == KEY_DOWN)
    {
      if (state & STATE_MORE)
      {
        skip += 16;
        window_invalid(NULL);
      }
      else if (selectidx < num_uids - 1)
      {
        selectidx++;
        fetch_content();
        window_invalid(NULL);
      }
    }
    else if (lparam == KEY_UP)
    {
      if (skip >= 16)
      {
        skip-=16;
        window_invalid(NULL);
      }
      else if (skip == 0)
      {
        if (selectidx > 0)
        {  
          selectidx--;
          fetch_content();
          window_invalid(NULL);
        }
     }
    }
    else if (lparam == KEY_ENTER)
    {
      if (selectidx < num_uids - 1)
      {
        selectidx++;
        fetch_content();
        window_invalid(NULL);
      }
    }
    break;
  default:
    return 0;
  }

  return 1;
}

