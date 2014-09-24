#include "msp430.h"
#include "contiki.h"
#include "isr_compat.h"
#include "rtc.h"
#include "system.h"
#include "window.h"
#include "btstack/include/btstack/utils.h"

PROCESS(rtc_process, "RTC Driver");
PROCESS_NAME(system_process);


#define now (globaldata.now)
static uint8_t source;

void rtc_init()
{
  // Configure RTC_A
  RTCCTL01 |= RTCHOLD + RTCMODE;
  // RTC enable, HEX mode, RTC hold
  // enable RTC time event interrupt

    // caculate the checksum
    RTCYEAR = 2014;                         // Year = 0x2010
    RTCMON = 4;                             // Month = 0x04 = April
    RTCDAY = 1;                            // Day = 0x05 = 5th
    RTCDOW = rtc_getweekday(14, 4, 1);
    RTCHOUR = 22;                           // Hour = 0x10
    RTCMIN = 10;                            // Minute = 0x32
    RTCSEC = 0;                            // Seconds = 0x45

  //  RTCADOWDAY = 0x2;                         // RTC Day of week alarm = 0x2
  //  RTCADAY = 0x20;                           // RTC Day Alarm = 0x20
  //  RTCAHOUR = 0x10;                          // RTC Hour Alarm
  //  RTCAMIN = 0x23;                           // RTC Minute Alarm

  process_start(&rtc_process, NULL);
  RTCCTL01 &= ~(RTCHOLD);                   // Start RTC calendar mode
}

PROCESS_THREAD(rtc_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    if (source == 0)
    {
      process_post(ui_process, EVENT_TIME_CHANGED, &now);
    }
  }
  PROCESS_END();
}

uint8_t rtc_getweekday(uint16_t year, uint8_t month, uint8_t day)
{
  if( month == 1 || month == 2 )
  {
    month += 12;
    year--;
  }

  return 1 + (( day + 2*month + 3*(month+1)/5 + year + year/4 ) %7);
}

void rtc_save()
{
  BUSYWAIT_UNTIL((RTCCTL01&RTCRDY), CLOCK_SECOND/8);
  now.minute = RTCMIN;
  now.second = RTCSEC;
}

void rtc_restore()
{
  RTCYEAR = now.year;
  RTCMON = now.month;
  RTCDAY = now.day;
  RTCDOW = rtc_getweekday(now.year - 2000, now.month, now.day);
  RTCHOUR = now.hour;
  RTCMIN = now.minute;
  RTCSEC = now.second;

  rtc_setalarm(now.aday, now.adow, now.ahour, now.aminute);
}

void rtc_setdate(uint16_t year, uint8_t month, uint8_t day)
{
  uint8_t weekday;

  weekday = rtc_getweekday(year, month, day);
  now.year = RTCYEAR = year;                         // Year = 0x2010
  now.month = RTCMON = month;                             // Month = 0x04 = April
  now.day = RTCDAY = day;                            // Day = 0x05 = 5th
  RTCDOW = weekday;                            // Day of week = 0x01 = Monday

  process_post(ui_process, EVENT_TIME_CHANGED, &now);
}

void rtc_settime(uint8_t hour, uint8_t min, uint8_t sec)
{
  now.hour = RTCHOUR = hour;                           // Hour = 0x10
  now.minute = RTCMIN = min;                            // Minute = 0x32
  now.second = RTCSEC = sec;                            // Seconds = 0x45

  process_post(ui_process, EVENT_TIME_CHANGED, &now);
}

uint8_t rtc_getmaxday(uint16_t year, uint8_t month)
{
  if (month == 2)
  {
    if ((year%4==0 && year%100==0) || year%400==0)
      return 29;
    else
      return 28;
  }
  if(month==8)
  {
    return 31;
  }

  if (month % 2 == 0)
  {
    return 30;
  }
  else
  {
    return 31;
  }
}

void rtc_setalarm(uint8_t aday, uint8_t adow, uint8_t ahour, uint8_t aminute)
{
  int enable = 0;
  RTCCTL0 &= ~RTCAIE;
  RTCCTL0 &= ~RTCAIFG;

  if (adow & 0x80) 
  {
    RTCADOW = adow;
    now.adow = adow;
    enable = 1;
  }

  if (aday & 0x80)
  {
   RTCADAY = aday;
   now.aday = aday;
   enable =1;
  }

  if (aminute & 0x80)
  {
    RTCAMIN = aminute;
    now.aminute = aminute;
    enable = 1;
  }
  
  if (ahour & 0x80)
  {
    RTCAHOUR = ahour;
    now.ahour = ahour;
    enable = 1;
  }

  if (enable)
  {
    RTCCTL0 |= RTCAIE;
  }
}

void rtc_readtime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
  BUSYWAIT_UNTIL((RTCCTL01&RTCRDY), CLOCK_SECOND/8);

  if (hour) *hour = RTCHOUR;
  if (min) *min = RTCMIN;
  if (sec) *sec = RTCSEC;
}

void rtc_readdate(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday)
{
  BUSYWAIT_UNTIL((RTCCTL01&RTCRDY), CLOCK_SECOND/8);

  if (year) *year = RTCYEAR;
  if (month) *month = RTCMON;
  if (day) *day = RTCDAY;
  if (weekday) *weekday = RTCDOW;
}

static const uint8_t month_day_map[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};
uint32_t calc_timestamp(uint8_t year, uint8_t month, uint8_t day, uint8_t hh, uint8_t mm, uint8_t ss)
{
    uint8_t leap_years = year / 4 + 1;
    if (year % 4 == 0 && month < 2)
        leap_years -= 1;

    uint32_t days = year * 365 + leap_years;
    for (uint8_t i = 0; i < month - 1; ++i)
        days += month_day_map[i];
    days += day;
    return ((days * 24 + hh) * 60 + mm) * 60 + ss;

}

void parse_timestamp(uint32_t time, uint8_t* year, uint8_t* month, uint8_t* day, uint8_t* hh, uint8_t* mm, uint8_t* ss)
{
    uint32_t temp = 0;

    *ss = time % 60;
    temp = time / 60;

    *mm = temp % 60;
    temp = temp / 60;

    *hh = temp % 24;
    temp = temp / 24;

    uint16_t total_day = temp;
    uint8_t tyear = 0;
    while(1)
    {
        if (tyear % 4 == 0)
        {
            if (total_day >= 366)
                total_day -= 366;
            else
                break;
        }
        else
        {
            if (total_day >= 365)
                total_day -= 365;
            else
                break;
        }
        tyear++;
    }

    uint8_t i = 0;
    for (; i < count_elem(month_day_map); ++i)
    {
        if (tyear % 4 == 0 && i == 1)
        {
            if (total_day < month_day_map[i] + 1)
                break;
            total_day -= month_day_map[i] + 1;
        }
        else
        {
            if (total_day < month_day_map[i])
                break;
            total_day -= month_day_map[i];
        }
    }

    *year  = tyear;
    *month = i + 1;
    *day   = total_day;

}

uint32_t rtc_readtime32()
{
    uint16_t year  = 0;
    uint8_t  month = 0;
    uint8_t  day   = 0;
    uint8_t  wday  = 0;
    rtc_readdate(&year, &month, &day, &wday);

    uint8_t  hour  = 0;
    uint8_t  min   = 0;
    uint8_t  sec   = 0;
    rtc_readtime(&hour, &min, &sec);
    return calc_timestamp(year - 2000, month, day, hour, min, sec);
}

void rtc_enablechange(uint8_t changes)
{
  if (changes & MINUTE_CHANGE)
  {
    RTCCTL01 |= RTCTEV__MIN + RTCTEVIFG + RTCTEVIE;
  }
  else
  {
    RTCCTL01 &= ~(RTCTEV__MIN + RTCTEVIE);
  }

  if (changes & SECOND_CHANGE)
  {
    RTCCTL01 |= RTCRDYIE + RTCRDYIFG;
  }
  else
  {
    RTCCTL01 &= ~RTCRDYIE;
  }

  if (changes & TENMSECOND_CHANGE)
  {
    RTCPS1CTL = RT1PSIE | RT1IP1;
  }
  else
  {
    RTCPS1CTL &= ~RT1PSIE;
  }
}

ISR(RTC, RTC_ISR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  switch(__even_in_range(RTCIV,16))
  {
  case RTC_NONE:                          // No interrupts
    break;
  case RTC_RTCRDYIFG:                     // RTCRDYIFG
    {
      source = 0;
      now.hour   = RTCHOUR;
      now.minute = RTCMIN;
      now.second = RTCSEC;
      now.year = RTCYEAR;
      now.month = RTCMON;
      now.day = RTCDAY;
      process_poll(&rtc_process);
      LPM4_EXIT;
      break;
    }
  case RTC_RTCTEVIFG:                     // RTCEVIFG
    {
      source = 0;
      now.hour   = RTCHOUR;
      now.minute = RTCMIN;
      now.second = RTCSEC;
      now.year = RTCYEAR;
      now.month = RTCMON;
      now.day = RTCDAY;
      process_poll(&rtc_process);
      LPM4_EXIT;
      break;
    }
  case RTC_RTCAIFG:                       // RTCAIFG
    {
      source = 1;
      process_poll(&rtc_process);
      LPM4_EXIT;
      break;
    }
  case RTC_RT0PSIFG:                      // RT0PSIFG
    break;
  case RTC_RT1PSIFG:                      // RT1PSIFG
    source = 0;
    process_poll(&rtc_process);
    LPM4_EXIT;
    break;
  case 12: break;                         // Reserved
  case 14: break;                         // Reserved
  case 16: break;                         // Reserved
  default: break;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
