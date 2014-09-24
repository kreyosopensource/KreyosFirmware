#include "contiki.h"
#include "window.h"
#include "rtc.h"
#include "grlib/grlib.h"
#include "cordic.h"
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "memlcd.h"
/*
* This implement the digit watch
* Wake up every 1 second and update the watch
* If in 10 minutes, no key or other things
* if get system key in non-suspend state, post event to system.
*/
#define CENTER_X (LCD_WIDTH/2)
#define CENTER_Y (LCD_HEIGHT/2)

#define MIN_HAND_LEN 50
#define HOUR_HAND_LEN 36

#define _hour d.analog.hour
#define _minute d.analog.minute
#define _sec d.analog.sec
static uint8_t selection;

extern uint8_t disable_key;

typedef void (*draw_function)(tContext *pContext);

static void drawFace0(tContext *pContext)
{
  int cos_val, sin_val;
  int sx, sy, ex, ey;

  for(int angle = 0; angle < 359; angle +=30)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    ex = CENTER_X + cordic_multipy(sin_val, CENTER_X - 20);
    ey = CENTER_Y - cordic_multipy(cos_val, CENTER_X - 20);
    sx = CENTER_X + cordic_multipy(sin_val, CENTER_X - 3);
    sy = CENTER_Y - cordic_multipy(cos_val, CENTER_X - 3);

    //tRect rect = {sx, sy, ex, ey};
    GrLineFill(pContext, sx, sy, ex, ey, 5);
  }
}

static void drawFace3(tContext *pContext)
{
  int cos_val, sin_val;
  int sx, sy, ex, ey;

  for(int angle = 0; angle < 359; angle += 6)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    sx = CENTER_X + ((52 * (sin_val >> 8)) >> 7);
    sy = CENTER_Y - ((52 * (cos_val >> 8)) >> 7);

    if (angle % 30 == 0)
    {
      ex = CENTER_X + ((64 * (sin_val >> 8)) >> 7);
      ey = CENTER_Y - ((64 * (cos_val >> 8)) >> 7);

      GrLineDraw(pContext, sx, sy, ex, ey);
    }
    else
    {
      GrCircleFill(pContext, sx, sy, 2);
    }
  }
}

static void drawFace6(tContext *pContext)
{
  int cos_val, sin_val;
  int x, y;
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 65);
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 66);

  for(int angle = 0; angle < 359; angle += 30)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    x = CENTER_X + cordic_multipy(sin_val, 60);
    y = CENTER_Y - cordic_multipy(cos_val, 60);

#if 0
    if (angle % 90 == 0)
    {
      GrCircleFill(pContext, x, y, 4);
    }
    else
#endif
    {
      GrCircleFill(pContext, x, y, 2);
    }
  }
}

static void drawFace7(tContext *pContext)
{
  int cos_val, sin_val;
  int x, y;
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 62);
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 63);

  for(int angle = 0; angle < 359; angle += 6)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    x = CENTER_X + cordic_multipy(sin_val, 57);
    y = CENTER_Y - cordic_multipy(cos_val, 57);

    GrCircleFill(pContext, x, y, 1);
  }
}

static void drawFace4(tContext *pContext)
{
  int cos_val, sin_val;
  int sx, sy, ex, ey;

  for(int angle = 0; angle < 359; angle += 6)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    sx = CENTER_X + ((64 * (sin_val >> 8)) >> 7);
    sy = CENTER_Y - ((64 * (cos_val >> 8)) >> 7);

    if (angle % 30 == 0)
    {
      ex = CENTER_X + ((53 * (sin_val >> 8)) >> 7);
      ey = CENTER_Y - ((53 * (cos_val >> 8)) >> 7);

      GrLineDraw(pContext, sx, sy, ex, ey);
    }
    else
    {
      GrCircleFill(pContext, sx, sy, 2);
    }
  }
}

static void drawFace1(tContext *pContext)
{
  int cos_val, sin_val;
  int x, y;

  for(int angle = 0; angle < 359; angle += 6)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    x = CENTER_X + ((69 * (sin_val >> 8)) >> 7);
    y = CENTER_Y - ((69 * (cos_val >> 8)) >> 7);

    if (angle % 30 == 0)
    {
      GrCircleFill(pContext, x, y, 3);
    }
    else
    {
      GrCircleFill(pContext, x, y, 1);
    }
  }
}

static void drawFace5(tContext *pContext)
{
  int cos_val, sin_val;
  int x, y;

  GrContextFontSet(pContext, &g_sFontNimbus30);

  for(int angle = 60; angle <= 360; angle += 60)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    x = CENTER_X + cordic_multipy(sin_val, 60);
    y = CENTER_Y - cordic_multipy(cos_val, 60);

    char buf[30];
    sprintf(buf, "%d", angle/30);
    GrStringDrawCentered(pContext, buf, -1, x, y, 0);
  }

  for(int angle = 30; angle <= 360; angle += 60)
  {
    cordic_sincos(angle, 13, &sin_val, &cos_val);
    x = CENTER_X + cordic_multipy(sin_val, 60);
    y = CENTER_Y - cordic_multipy(cos_val, 60);

    GrCircleFill(pContext, x, y, 3);
  }
}


// design 3, hand
static void drawHand0(tContext *pContext)
{
  int cos_val, sin_val;
  int angle;
  int x, y;

  // minute hand = length = 70
  angle = _minute * 6+ _sec /10;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  x = CENTER_X + ((MIN_HAND_LEN * (sin_val >> 8)) >> 7);
  y = CENTER_Y - ((MIN_HAND_LEN * (cos_val >> 8)) >> 7);
  GrContextForegroundSet(pContext, ClrWhite);
  GrLineDraw(pContext, CENTER_X, CENTER_Y,  x, y);

  // hour hand 45
  angle = _hour * 30 + _minute / 2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  x = CENTER_X + ((HOUR_HAND_LEN * (sin_val >> 8)) >> 7);
  y = CENTER_Y - ((HOUR_HAND_LEN * (cos_val >> 8)) >> 7);
  GrContextForegroundSet(pContext, ClrWhite);
  GrLineFill(pContext, CENTER_X, CENTER_Y,  x, y, 2);
}

static void drawHand1(tContext *pContext)
{
  int cos_val, sin_val;
  int angle;
  int sx, sy, ex, ey;

  // draw the circle
  GrCircleFill(pContext, CENTER_X, CENTER_Y, 3);
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 9);
  GrCircleDraw(pContext, CENTER_X, CENTER_Y, 10);

  // minute hand = length = 70
  angle = _minute * 6+ _sec /10;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + ((MIN_HAND_LEN * (sin_val >> 8)) >> 7);
  ey = CENTER_Y - ((MIN_HAND_LEN * (cos_val >> 8)) >> 7);
  sx = CENTER_X + ((14 * (sin_val >> 8)) >> 7);
  sy = CENTER_Y - ((14 * (cos_val >> 8)) >> 7);
  GrContextForegroundSet(pContext, ClrWhite);
  GrLineFill(pContext, sx, sy,  ex, ey, 4);

  // hour hand 45
  angle = _hour * 30 + _minute / 2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + ((HOUR_HAND_LEN * (sin_val >> 8)) >> 7);
  ey = CENTER_Y - ((HOUR_HAND_LEN * (cos_val >> 8)) >> 7);
  sx = CENTER_X + ((14 * (sin_val >> 8)) >> 7);
  sy = CENTER_Y - ((14 * (cos_val >> 8)) >> 7);
  GrContextForegroundSet(pContext, ClrWhite);
  GrLineFill(pContext, sx, sy,  ex, ey, 3);
}

static void drawHand2(tContext *pContext)
{
  GrCircleFill(pContext, CENTER_X, CENTER_Y, 12);

  drawHand0(pContext);
}

static void drawHand4(tContext *pContext)
{
  int cos_val, sin_val;
  int angle;
  int sx, sy, ex, ey, mx, my;

  // degree is caculated by 
  // arctan(35/7)=?deg

  // hour hand 45
  angle = _hour * 30 + _minute / 2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  mx = CENTER_X + cordic_multipy(sin_val, 37);
  my = CENTER_Y - cordic_multipy(cos_val, 37);

  angle = angle - 79;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  sx = CENTER_X + cordic_multipy(sin_val, 7);
  sy = CENTER_Y - cordic_multipy(cos_val, 7);

  angle = angle + 79*2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + cordic_multipy(sin_val, 7);
  ey = CENTER_Y - cordic_multipy(cos_val, 7);
  // draw a filled triagle
  GrTriagleFill(pContext, 
    ex, ey,
    sx, sy,
    mx, my
    );

  // draw a black
  GrContextForegroundSet(pContext, ClrBlack);
  GrCircleFill(pContext, CENTER_X, CENTER_Y, 8);
  GrContextForegroundSet(pContext, ClrWhite);

  // minute hand
  angle = _minute * 6+ _sec /10;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  mx = CENTER_X + cordic_multipy(sin_val, 59);
  my = CENTER_Y - cordic_multipy(cos_val, 59);

  angle = angle - 84;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  sx = CENTER_X + cordic_multipy(sin_val, 7);
  sy = CENTER_Y - cordic_multipy(cos_val, 7);

  angle = angle + 84*2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + cordic_multipy(sin_val, 7);
  ey = CENTER_Y - cordic_multipy(cos_val, 7);
  // draw a filled triagle
  GrTriagleFill(pContext, 
    ex, ey,
    sx, sy,
    mx, my
    );

  GrCircleFill(pContext, CENTER_X, CENTER_Y, 7);
}



static void drawHand5(tContext *pContext)
{
  int cos_val, sin_val;
  int angle;
  int sx, sy, ex, ey, mx, my;

  // degree is caculated by 
  // arctan(35/7)=?deg

  // hour hand 45
  angle = _hour * 30 + _minute / 2;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  mx = CENTER_X + cordic_multipy(sin_val, 53);
  my = CENTER_Y - cordic_multipy(cos_val, 53);

  angle = angle - 150;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  sx = CENTER_X + cordic_multipy(sin_val, 13);
  sy = CENTER_Y - cordic_multipy(cos_val, 13);

  angle = angle + 300;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + cordic_multipy(sin_val, 13);
  ey = CENTER_Y - cordic_multipy(cos_val, 13);

  GrLineDraw(pContext, ex, ey, sx, sy);
  GrLineDraw(pContext, mx, my, sx, sy);
  GrLineDraw(pContext, ex, ey, mx, my);

  // minute hand
  angle = _minute * 6+ _sec /10;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  mx = CENTER_X + cordic_multipy(sin_val, 65);
  my = CENTER_Y - cordic_multipy(cos_val, 65);

  angle = angle - 150;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  sx = CENTER_X + cordic_multipy(sin_val, 12);
  sy = CENTER_Y - cordic_multipy(cos_val, 12);

  angle = angle + 300;
  cordic_sincos(angle, 13, &sin_val, &cos_val);
  ex = CENTER_X + cordic_multipy(sin_val, 12);
  ey = CENTER_Y - cordic_multipy(cos_val, 12);

  GrLineDraw(pContext, ex, ey, sx, sy);
  GrLineDraw(pContext, mx, my, sx, sy);
  GrLineDraw(pContext, ex, ey, mx, my);

  GrPixelDraw(pContext, CENTER_X, CENTER_Y);
}

struct clock_draw {
  draw_function faceDraw;
  draw_function handDraw;
}FaceSelections[] =
{
  {drawFace5, drawHand4},
  {drawFace5, drawHand5},
  {drawFace5, drawHand0},
  {drawFace0, drawHand4},
  {drawFace0, drawHand5},
  {drawFace0, drawHand0},
  {drawFace6, drawHand4},
  {drawFace6, drawHand5},
  {drawFace6, drawHand0},
};

uint8_t analogclock_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  if (ev == EVENT_WINDOW_CREATED)
  {
    rtc_readtime(&_hour, &_minute, &_sec);
    if (rparam == NULL)
      selection = window_readconfig()->analog_clock;
    else
      selection = (uint8_t)rparam - 0x1;
    if (selection > sizeof(FaceSelections)/sizeof(struct clock_draw) - 1)
      selection = sizeof(FaceSelections)/sizeof(struct clock_draw) - 1;

    rtc_enablechange(MINUTE_CHANGE);

    return 0x80;
  }
  else if (ev == EVENT_WINDOW_ACTIVE)
  {
    rtc_readtime(&_hour, &_minute, &_sec);
  }
  else if (ev == EVENT_WINDOW_PAINT)
  {
    tContext *pContext = (tContext*)rparam;
    GrContextForegroundSet(pContext, ClrBlack);
    GrRectFill(pContext, &fullscreen_clip);

    GrContextForegroundSet(pContext, ClrWhite);

    FaceSelections[selection].faceDraw(pContext);
    FaceSelections[selection].handDraw(pContext);
  }
  else if (ev == EVENT_TIME_CHANGED)
  {
    struct datetime* dt = (struct datetime*)rparam;
    _hour = dt->hour;
    _minute = dt->minute;
    _sec = dt->second;
    window_invalid(NULL);
  }
  else if ((ev == EVENT_KEY_PRESSED) && (disable_key == 0))
  {
    if (lparam == KEY_DOWN)
    {
      selection += 0x1;
      if (selection > sizeof(FaceSelections)/sizeof(struct clock_draw) - 1)
      {
        selection = 0x00;
      }
      window_invalid(NULL);
    }
    else if (lparam == KEY_UP)
    {
      selection -= 0x1;
      if (selection == 0xff)
      {
        selection = sizeof(FaceSelections)/sizeof(struct clock_draw) - 1;
      }
      window_invalid(NULL);
    }
  }
  else if (ev == EVENT_WINDOW_CLOSING)
  {
    rtc_enablechange(0);

    window_readconfig()->default_clock = 0;
    if (selection != window_readconfig()->analog_clock)
    {
      window_readconfig()->analog_clock = selection;
      window_writeconfig();
    }
  }
  else
  {
    return 0;
  }

  return 1;
}
