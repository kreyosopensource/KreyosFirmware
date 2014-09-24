
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "status.h"
#include "contiki.h"
#include "sys/ctimer.h"
#include "grlib/grlib.h"
#include "window.h"
#include "bluetooth.h"
#include "battery.h"
#include "rtc.h"
#include "pedometer/pedometer.h"
#include "sportsdata.h"
#include "memory.h"
#include "memlcd.h"
#include "icons.h"

extern const tRectangle status_clip;
extern void hfp_battery(int level);
extern void ped_reset();

#define CHARGE_X 137
#define ICONSPACE 18

#define BATTERY_STATUS 0x07 // bit0 bit1 bit2 for battery
#define BLUETOOTH_STATUS 0x08
#define ANTPLUS_STATUS 0x10
#define ALARM_STATUS  0x20
#define MID_STATUS    0x40

#define BATTERY_EMPTY 0
#define BATTERY_LESS  1
#define BATTERY_MORE  2
#define BATTERY_FULL  3
#define BATTERY_CHARGING 4

static uint16_t status;
static char alarmtext[10]; // 12:34 67

extern void adjustAMPM(uint8_t hour, uint8_t *outhour, uint8_t *ispm);
extern void cleanUpSportsWatchData();

static uint16_t s_watch_status = 0;
uint16_t add_watch_status(uint16_t value)
{
  uint16_t invalue = get_watch_status() | value;
  return set_watch_status(invalue);
}

uint16_t del_watch_status(uint16_t value)
{
  uint16_t invalue = get_watch_status() & (~value);
  return set_watch_status(invalue);
}

uint16_t set_watch_status(uint16_t value)
{
  uint16_t oldvalue = s_watch_status;
  s_watch_status = value;
  return oldvalue;
}

uint16_t get_watch_status()
{
  return s_watch_status;
}

static uint16_t s_last_act_timehash = 0;

static uint16_t get_time_hash(uint16_t hour, uint16_t min, uint16_t sec)
{
  return hour * 60 * 60 + min * 60 + sec;
}

void record_last_action()
{
  uint8_t hour, min, sec;
  rtc_readtime(&hour, &min, &sec);
  s_last_act_timehash = get_time_hash(hour, min, sec);
}

uint16_t check_idle_time()
{
  uint8_t hour, min, sec;
  rtc_readtime(&hour, &min, &sec);
  uint16_t now = get_time_hash(hour, min, sec);
  return now - s_last_act_timehash;
}

void adjustAMPM(uint8_t hour, uint8_t *outhour, uint8_t *ispm);

static void OnDraw(tContext* pContext)
{
  // clear the region
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &status_clip);

  GrContextForegroundSet(pContext, ClrWhite);
  GrLineDrawH(pContext, 0, LCD_WIDTH, 16);

  GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);
  char icon;
  int x = CHARGE_X;

  switch(status & 0x03)
  {
    case BATTERY_EMPTY:
      icon = ICON_BATTERY_EMPTY;
      break;
    case BATTERY_LESS:
      icon = ICON_BATTERY_LESS;
      break;
    case BATTERY_MORE:
      icon = ICON_BATTERY_MORE;
      break;
    case BATTERY_FULL:
      icon = ICON_BATTERY_FULL;
      break;
    default:
      icon = 0;
  }

  if (status & BATTERY_CHARGING)
  {
    GrStringDraw(pContext, &icon, 1, 120, 0, 0);
    icon = ICON_CHARGING;
    GrStringDraw(pContext, &icon, 1, 137, 0, 0);
  }
  else
  {
    GrStringDraw(pContext, &icon, 1, 127, 0, 0);
  }

  x = 2;
  if (status & ALARM_STATUS)
  {
    icon = ICON_ALARM;
    GrStringDraw(pContext, &icon, 1, x, 0, 0);
    x += 15;
  }

  if (status & BLUETOOTH_STATUS)
  {
    icon = ICON_BT;
    GrStringDraw(pContext, &icon, 1, x, 0, 0);
  }

  if (status & MID_STATUS)
  {
    char icon = ICON_RUN;
    // draw activity
    GrContextFontSet(pContext, (tFont*)&g_sFontExIcon16);
    GrStringDraw(pContext, &icon, 1, 48, 0, 0);

    uint16_t part = window_readconfig()->goal_steps / 5;
    uint16_t steps = ped_get_steps();
    for(int i = 0; i < 5; i++)
    {
      if (i * part + part / 2 <= steps)
      {
        GrCircleFill(pContext, 68 + i*6, 7, 2);
      }
      else
      {
        GrCircleDraw(pContext, 68 + i*6, 7, 2);
      }
    }
  }
  else
  {
    uint8_t hour, minute;
    char buf[20];
    uint8_t ispm;
    rtc_readtime(&hour, &minute, NULL);

    adjustAMPM(hour, &hour, &ispm);

    sprintf(buf, "%02d:%02d %s", hour, minute, ispm?"PM":"AM");
    GrContextFontSet(pContext, &g_sFontGothic14);
    GrStringDrawCentered(pContext, buf, -1, LCD_WIDTH/2, 8, 0);
  }
}

/*
 battery is between 3.7 to 4.2
  4.2 => 2.1 / 2.5 * 255 = 214
  3.7 => 1.86 / 2.5 * 255 = 189
*/
static void check_battery()
{
  //uint8_t report = 0;
  // update battery status
  BATTERY_STATE state = battery_state();
  uint8_t level = battery_level(state);

  status &= ~BATTERY_STATUS;

  switch(level)
  {
    case 0: case 1:
    status |= BATTERY_EMPTY;
    break;
    case 2: case 3:
    status |= BATTERY_LESS;
    break;
    case 4: case 5: case 6:
    status |= BATTERY_MORE;
    break;
    default:
    status |= BATTERY_FULL;
  }

  if (window_current() != &charging_process)
  {
    if ((level == 0) && (state == BATTERY_STATE_DISCHARGING))
    {
      window_open(&charging_process, 0);
    }
  }

#ifndef UNITTEST
  if (window_current() == &menu_process ||
    window_current() == &analogclock_process ||
    window_current() == &digitclock_process)
  {
    if (state != BATTERY_STATE_DISCHARGING)
    {
      window_open(&charging_process, 0); 
    }
  }
#endif

  if (state == BATTERY_STATE_CHARGING)
  {
    status |= BATTERY_CHARGING;
    level |= 0x10;
  }

  hfp_battery(level);
}

static uint32_t s_daily_data[3] = {0};
static uint8_t s_cur_min = 0;

static void on_midnigth(uint8_t event, uint16_t lparam, void* rparam)
{
  ped_reset();

  memset(&s_daily_data, 0, sizeof(s_daily_data));

  uint16_t year;
  uint8_t month, day, weekday;
  rtc_readdate(&year, &month, &day, &weekday);
  create_data_file(year - 2000, month, day);

  cleanUpSportsWatchData();

}

static void record_activity_data(uint8_t hour, uint8_t minute)
{
  uint32_t data[3] = {0};
  data[0] = ped_get_steps()    - s_daily_data[0];
  data[1] = ped_get_calorie()  - s_daily_data[1];
  data[2] = ped_get_distance() - s_daily_data[2];

  if (data[0] != 0 || data[1] != 0 || data[2] != 0)
  {
    uint8_t meta[] = {DATA_COL_STEP, DATA_COL_CALS, DATA_COL_DIST};
    write_data_line(get_mode(), hour, minute, meta, data, sizeof(data) / sizeof(data[0]));

    s_daily_data[0] += data[0];
    s_daily_data[1] += data[1];
    s_daily_data[2] += data[2];
  }
}

uint8_t status_process(uint8_t event, uint16_t lparam, void* rparam)
{
  uint8_t old_status = status;
  switch(event)
  {
  case EVENT_WINDOW_CREATED:
    status = 0;
    check_battery();
    status_invalid();
    break;
  case EVENT_WINDOW_PAINT:
    OnDraw((tContext*)rparam);
    break;
  case PROCESS_EVENT_TIMER:
    {
      uint8_t hour, minute, second;
      rtc_readtime(&hour, &minute, &second);

      if (hour == 0 && minute == 0 && second <= 30)
      {
        on_midnigth(event, lparam, rparam);
      }

      if (s_cur_min != minute &&
        (get_mode() == DATA_MODE_NORMAL || (get_mode() & DATA_MODE_PAUSED) != 0))
      {
        s_cur_min = minute;
        record_activity_data(hour, minute);
      }

#if 0
      //sports watch pause/resume
      uint16_t ws_status = get_watch_status();
      if (ws_status != WS_NORMAL)
      {
        if (check_idle_time() > 60)
        {
          if (ws_status & WS_NOTIFY)
          {
            //TODO: close current front-end window
          }
          else if (ws_status & WS_SPORTS)
          {
            //TODO: open sports watch
          }
        }
      }
#endif

      //if (minute % 5 == 0)
      check_battery();

      //check alarms
      ui_config* uiconf = window_readconfig();
      for (int i = 0; i < sizeof(uiconf->alarms) / sizeof(uiconf->alarms[0]); ++i)
      {
        if (uiconf->alarms[i].flag != 0 &&
            uiconf->alarms[i].hour    == hour &&
            uiconf->alarms[i].minutes == minute &&
            second < 30)
        {
          uint8_t ispm;
          adjustAMPM(hour, &hour, &ispm);
          sprintf(alarmtext, "%02d:%02d %s", hour, minute, ispm?"PM":"AM");
          window_messagebox(ICON_LARGE_ALARM, alarmtext, NOTIFY_ALARM);
          break;
        }
      }

      status ^= MID_STATUS;
      break;
    }
  case EVENT_BT_STATUS:
    if (lparam == BT_CONNECTED)
      status |= BLUETOOTH_STATUS;
    else if (lparam == BT_DISCONNECTED)
      status &= ~BLUETOOTH_STATUS;
    break;
  case EVENT_ANT_STATUS:
    break;
  default:
    return 0;
  }

  if (status != old_status)
    status_invalid();
  return 1;
}
