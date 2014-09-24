#include "window"

static day
static month
static year
static hour, minute, second

refreshdata()
{
	getdate(year, month, day)
	gettime(hour, minute, second)
	window_invalid()
}

@oncreate()
{
	refreshdata();

	enableclock(Minute)
}

@onpaint(context: context)
{
    new command{30}
	new ampm = 0
	new width

	if (hour > 12)
	{
		ampm = 1;
		hour = hour - 12;
	}

	window_setfont context, FontExNimbus46
	strformat command, _, true, "%02d:%02d", hour, minute
	width = window_getwidth(context, command)
	window_drawtext context, command, (LCD_WIDTH - width)/2, 70, 0

	window_setfont context, FontGothic14
	if (ampm)
		window_drawtext context, "PM", 105, 105, 0
	else
		window_drawtext context, "AM", 105, 105, 0

	window_setfont context, FontGothic14
	strformat command, _, true, "%d %d, %d", month, day, year
	width = window_getwidth(context, command)
	window_drawtext context, command, (LCD_WIDTH - width)/2, 35, 0

	window_drawtext context, "from script", (LCD_WIDTH - width)/2, 115, 0
}

@onclock()
{
  refreshdata()
}

