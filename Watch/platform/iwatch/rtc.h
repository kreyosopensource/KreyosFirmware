#ifndef _RTC_H_
#define _RTC_H_
#include <stdint.h>

struct datetime{
  uint16_t year;
  uint8_t  month;
  uint8_t  day;

  uint8_t  hour;
  uint8_t  minute;
  uint8_t  second;

  uint8_t ahour, aminute, aday, adow;
};

extern void rtc_init();
extern void rtc_setdate(uint16_t year, uint8_t month, uint8_t day);
extern void rtc_settime(uint8_t hour, uint8_t min, uint8_t sec);
extern void rtc_readtime(uint8_t *hour, uint8_t *min, uint8_t *sec);
extern uint32_t rtc_readtime32();
extern void rtc_readdate(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday);
extern uint8_t rtc_getmaxday(uint16_t year, uint8_t month);
extern uint8_t rtc_getweekday(uint16_t year, uint8_t month, uint8_t day);
extern void rtc_setalarm(uint8_t aday, uint8_t adow, uint8_t ahour, uint8_t aminute);
extern void rtc_save();
extern void rtc_restore();
extern uint32_t calc_timestamp(uint8_t year, uint8_t month, uint8_t day, uint8_t hh, uint8_t mm, uint8_t ss);
extern void parse_timestamp(uint32_t time, uint8_t* year, uint8_t* month, uint8_t* day, uint8_t* hh, uint8_t* mm, uint8_t* ss);

#define SECOND_CHANGE 0x01
#define MINUTE_CHANGE 0x02
#define TENMSECOND_CHANGE 0x04
extern void rtc_enablechange(uint8_t changes);

#endif
