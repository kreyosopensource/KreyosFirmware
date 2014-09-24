#pragma library Window

const {
   LCD_WIDTH = 144,
   LCD_HEIGHT = 168
}

const clocktype: {
  Second = 1,
  Minute = 2,
  TenMS = 4
}

const color:
{
  White = 0,
  Black = 65535
}

const fonts: {
 FontGothic14 = 0,
 FontGothic18,
 FontGothic18b,
 FontGothic24b,
 FontGothic28,
 FontGothic28b,
 FontExIcon16,
 FontExIcon32,
 FontExIcon48,
 FontNimbus30,
 FontNimbus34,
 FontExNimbus38,
 FontExNimbus40,
 FontExNimbus46,
 FontExNimbus50,
 FontExNimbus52,
 FontExNimbus91,
 FontUnicode,
}

forward @oncreate();
forward @onclock();
forward @onpaint(context: context);
forward @onkey(key);
forward @onclose();

native strformat(dest[], size=sizeof dest, bool:pack=true, const format[], {Fixed,Float,_}:...) = -1;

native enableclock(clocktype: t) = -10
native gettime(&hour=0, &minute=0, &second=0) = -11
native getdate(&year=0, &month=0, &day=0) = -12

native window_invalid() = -20
native window_invalidrect(x, y, w, h) = -21
native window_setfont(context: context, fonts: font) = -22
native window_getwidth(context: context, string[]) = -23
native window_getheight(context: context) = -24
native window_setcolor(context: context, fg, bg) = -25
native window_centertext(context, string[], x, y, style) = -26
native window_drawtext(context, string[], x, y, style) = -27

native window_drawpixel(context: context, xa, ya) = -30
native window_drawline(context: context, xa, ya, xb, yb) = -31
native window_fillline(context: context, xa, ya, xb, yb, width) = -32
native window_drawcircle(context: context, x, y, r) = -33
native window_fillcircle(context: context, x, y, r) = -34
native window_drawtriagle(context: context, xa, ya, xb, yb, xc, yc) = -35
native window_filltriagle(context: context, xa, ya, xb, yb, xc, yc) = -36
native window_drawrect(context: context, xa, ya, xb, yb) = -37
native window_fillrect(context: context, xa, ya, xb, yb) = -38


