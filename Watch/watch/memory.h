#ifndef _MEMORY_H
#define _MEMORY_H
#include  <stdint.h>
#include "window.h"
#include "sportsdata.h"

extern union _data
{
  struct
  {
    uint8_t hour, minute, sec;
  }analog;

  struct
  {
    uint8_t hour0, minute;
  }digit;

  struct
  {
    uint8_t state;
    uint8_t times[3];
    uint32_t totaltime, lefttime;
  }countdown;

  struct
  {
    uint8_t state;
    uint8_t t[3];
  }config;

  struct
  {
    uint8_t state;
  }today;

  struct
  {
    const char *title;
    char titlebuf[80];
    const char *artist;
    char artistbuf[80];
    uint16_t length;
    uint16_t position;
  }music;

  struct
  {
    uint8_t state;
  }host;

  struct
  {
    uint8_t index;
  }world;

  struct
  {
    uint8_t state;
    uint8_t times[3]; // minute, second, 10ms
    uint8_t counter;
    uint8_t currentStop;
    uint8_t topView;
    uint8_t saved_times[15][3]; // saved time
    int8_t delta_times[15][3]; // delta
  }stop;

}d;

#endif
