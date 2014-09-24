
#include "osdefs.h"
#include "amx.h"

#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "rtc.h"

static const tFont* fonts[] =
{
 &g_sFontGothic14,
 &g_sFontGothic18,
 &g_sFontGothic18b,
 &g_sFontGothic24b,
 &g_sFontGothic28,
 &g_sFontGothic28b,
(tFont*)&g_sFontExIcon16,
(tFont*)&g_sFontExIcon32,
(tFont*)&g_sFontExIcon48,
 &g_sFontNimbus30,
 &g_sFontNimbus34,
(tFont*)&g_sFontExNimbus38,
(tFont*)&g_sFontExNimbus40,
(tFont*)&g_sFontExNimbus46,
(tFont*)&g_sFontExNimbus50,
(tFont*)&g_sFontExNimbus52,
(tFont*)&g_sFontExNimbus91,
(tFont*)&g_sFontUnicode,
};

static cell AMX_NATIVE_CALL n_invalid(AMX *amx,const cell *params)
{
  window_invalid(NULL);

  return 0;
}

static cell AMX_NATIVE_CALL n_invalid_rect(AMX *amx,const cell *params)
{
  tRectangle rect;
  rect.sXMin = params[1];
  rect.sYMin = params[2];
  rect.sXMax = params[1] + params[3];
  rect.sYMax = params[2] + params[4];

  window_invalid(&rect);

  return 0;
}

static cell AMX_NATIVE_CALL n_setfont(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  uint8_t font = (uint8_t)params[2];

  if (font < sizeof(fonts)/sizeof(tFont*))
    GrContextFontSet(context, fonts[font]);
  else
    GrContextFontSet(context, fonts[0]);
  return 0;
}

//native window_getwidth(context, string[])
static cell AMX_NATIVE_CALL n_getwidth(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  char *text;

  amx_StrParam(amx, params[2], text);

  return (cell)GrStringWidthGet(context, text, -1);
}

//native window_getheight(context)
static cell AMX_NATIVE_CALL n_getheight(AMX *amx, const cell *params)
{
  tContext *context = window_context();

  return (cell)GrStringHeightGet(context);
}

// window_drawtext(context, string[], x, y, style)
static cell AMX_NATIVE_CALL n_drawtext(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  char *text;

  amx_StrParam(amx, params[2], text);

  GrStringDraw(context, text, -1, params[3], params[4], params[5]);

  return 0;
}

// window_drawtext_center(context, string[], x, y, style)
static cell AMX_NATIVE_CALL n_drawtextcentered(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  char *text;

  amx_StrParam(amx, params[2], text);

  GrStringDrawCentered(context, text, -1, params[3], params[4], params[5]);

  return 0;
}

static cell AMX_NATIVE_CALL n_setcolor(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  
  GrContextForegroundSet(context, params[2]);
  GrContextBackgroundSet(context, params[3]);

  return 0;
}

static cell AMX_NATIVE_CALL n_gettime(AMX *amx, const cell *params)
{
  cell *cptr;

  printf("n_gettime called\n");
  assert(params[0]==(int)(3*sizeof(cell)));

  uint8_t hour, minute, second;
  rtc_readtime(&hour, &minute, &second);

  cptr=amx_Address(amx,params[1]);
  *cptr=hour;
  cptr=amx_Address(amx,params[2]);
  *cptr=minute;
  cptr=amx_Address(amx,params[3]);
  *cptr=second;

  return 0;
}

static cell AMX_NATIVE_CALL n_getdate(AMX *amx, const cell *params)
{
  cell *cptr;

  printf("n_getdate called\n");
  assert(params[0]==(int)(3*sizeof(cell)));

  uint16_t year;
  uint8_t month, day;
  rtc_readdate(&year, &month, &day, NULL);

  cptr=amx_Address(amx,params[1]);
  *cptr=year;
  cptr=amx_Address(amx,params[2]);
  *cptr=month;
  cptr=amx_Address(amx,params[3]);
  *cptr=day;

  return 0;
}

static cell AMX_NATIVE_CALL n_enableclock(AMX *amx, const cell *params)
{
  printf("n_enableclock %x called\n", params[1]);
  rtc_enablechange(params[1]);

  return 0;
}

static cell AMX_NATIVE_CALL n_drawcircle(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrCircleDraw(context, params[1], params[2], params[3]);

  return 0;
}

static cell AMX_NATIVE_CALL n_fillcircle(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrCircleFill(context, params[1], params[2], params[3]);

  return 0;
}

static cell AMX_NATIVE_CALL n_drawrect(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  tRectangle rect = {
    params[1], params[2], params[3], params[4]
  };
  GrRectDraw(context, &rect);

  return 0;
}

static cell AMX_NATIVE_CALL n_fillrect(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  tRectangle rect = {
    params[1], params[2], params[3], params[4]
  };
  GrRectFill(context, &rect);

  return 0;
}

static cell AMX_NATIVE_CALL n_drawpixel(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrPixelDraw(context, params[1], params[2]);

  return 0;
}

static cell AMX_NATIVE_CALL n_drawline(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrLineDraw(context, params[1], params[2], params[3], params[4]);

  return 0;
}

static cell AMX_NATIVE_CALL n_fillline(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrLineFill(context, params[1], params[2], params[3], params[4], params[5]);

  return 0;
}

static cell AMX_NATIVE_CALL n_drawtriagle(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrTriagleDraw(context, params[1], params[2], params[3], params[4], params[5], params[6]);

  return 0;
}

static cell AMX_NATIVE_CALL n_filltriagle(AMX *amx, const cell *params)
{
  tContext *context = window_context();
  GrTriagleFill(context, params[1], params[2], params[3], params[4], params[5], params[6]);

  return 0;
}

extern cell AMX_NATIVE_CALL n_strformat(AMX *amx,const cell *params);

AMX_NATIVE const window_natives[] =
{
  n_strformat, // -1
  NULL,
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 

  n_enableclock, // -10
  n_gettime, // -11
  n_getdate, // -12
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 
  NULL, 

  n_invalid, // -20
  n_invalid_rect, // -21
  n_setfont, // -22
  n_getwidth, // -23
  n_getheight, // -24
  n_setcolor,  // -25
  n_drawtextcentered, // -26
  n_drawtext, // -27
  NULL, 
  NULL, 

  n_drawpixel, // 30
  n_drawline, // 31
  n_fillline, // 32
  n_drawcircle, // 33
  n_fillcircle, // 34
  n_drawtriagle, // 35
  n_filltriagle, // 36
  n_drawrect, //37
  n_fillrect, //38
};


