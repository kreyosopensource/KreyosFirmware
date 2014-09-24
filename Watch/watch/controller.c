#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "avctp.h"
#include "avrcp.h"
#include "hfp.h"
#include "memory.h"
#include "gesture/gesture.h"
#include "icons.h"

#include <stdio.h>
#include <string.h>

static uint8_t state;

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define STATE_X 10
#define STATE_Y 38

#define TITLE_X 10
#define TITLE_Y 68

#define PROGRESS_Y 80

#define title d.music.title

#define titlebuf d.music.titlebuf
#define artist d.music.artist
#define artistbuf d.music.artistbuf
#define length d.music.length
#define position d.music.position

static const tRectangle rect = {12, 24, 144, 88};

static void OnDraw(tContext *pContext)
{
  // clear the region
  GrContextForegroundSet(pContext, ClrBlack);
  GrContextBackgroundSet(pContext, ClrWhite);
  GrRectFill(pContext, &client_clip);

  GrContextFontSet(pContext, &g_sFontGothic28b);
  // draw length
  //if (position != 0)
  {
    uint8_t times[3];
    uint16_t left = position;
    times[2] = left % 60;
    left /= 60;
    times[1] = left % 60;
    times[0] = left / 60;

    //GrContextForegroundSet(pContext, ClrWhite);
    //tRectangle rect = {12, 24, 132, 60};
    //GrRectFillRound(pContext, &rect, 3);

    GrContextForegroundSet(pContext, ClrWhite);
    window_drawtime(pContext, 44, times, 0);
#if 0

    // draw balls
    long r;
    long startx = -130 + (position * 143/ length);
    for(int i = 0; i < 11; i++)
    {
      r = i/2 + 1;
      GrCircleFill(pContext, startx, 22, r);
      startx += r * 2 + 3;
    }
    for(int i = 10; i >0; i--)
    {
      r = i/2 + 1;
      GrCircleFill(pContext, startx, 22, r);
      startx += r * 2 + 3;
    }
#endif
  }

  // draw title
  GrContextFontSet(pContext, (tFont*)&g_sFontUnicode);
  GrContextForegroundSet(pContext, ClrWhite);
  if (title)
    GrStringDraw(pContext, title, -1, 12, 108, 0);
  if (artist)
    GrStringDraw(pContext, artist, -1, 12, 125, 0);

  switch(state)
  {
  case AVRCP_PLAY_STATUS_ERROR:
    window_button(pContext, KEY_UP, NULL);
    window_button(pContext, KEY_DOWN, NULL);
    window_button(pContext, KEY_ENTER, NULL);
    break;
  case AVRCP_PLAY_STATUS_PAUSED:
  case AVRCP_PLAY_STATUS_STOPPED:
    window_button(pContext, KEY_UP, "PLAY");
    window_button(pContext, KEY_DOWN, "NEXT");
    window_button(pContext, KEY_ENTER, "PREV");
    break;
  case AVRCP_PLAY_STATUS_PLAYING:
    window_button(pContext, KEY_UP, "PAUSE");
    window_button(pContext, KEY_DOWN, "NEXT");
    window_button(pContext, KEY_ENTER, "PREV");
    break;
  }
}

void handle_av_events(uint16_t lparam, void* rparam)
{
    switch(lparam)
    {
    case EVENT_AV_CONNECTED:
      break;
    case EVENT_AV_DISCONNECTED:
       break;
    case EVENT_AV_STATUS:
        if (state != (int)rparam)
        {
          avrcp_get_attributes(0);

          state = (int)rparam;
          if (state == AVRCP_PLAY_STATUS_PLAYING)
          {
            PRINTF("enable timer\n");
            window_timer(CLOCK_SECOND);
          }
          else
          {
            PRINTF("disable timer :%d\n", state);
            window_timer(0);
          }
          window_invalid(NULL);
        }
        break;
    case EVENT_AV_TITLE:
        strncpy(titlebuf, rparam, sizeof(titlebuf));
        title = titlebuf;
        break;
    case EVENT_AV_ARTIST:
        strncpy(artistbuf, rparam, sizeof(artistbuf));
        artist = artistbuf;
        window_invalid(NULL);
        break;
    case EVENT_AV_LENGTH:
        length = (uint16_t)((uint32_t)rparam/1000);
        PRINTF("length set to %d\n", length);
        break;
    case EVENT_AV_POS:
        position = (uint16_t)((uint32_t)rparam/1000);
        PRINTF("position set to %d\n", position);
        window_invalid(&rect);
        break;
    case EVENT_AV_TRACK:
        position = 0;
        state = AVRCP_PLAY_STATUS_ERROR;
        avrcp_get_playstatus();
        break;
    }
}

uint8_t control_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev){
  case EVENT_WINDOW_CREATED:
  {
      if (!hfp_connected())
      {
        window_messagebox(ICON_LARGE_WARNING, PairingWarning, 0);
        return 1;
      }
      break;
  }
  case EVENT_WINDOW_ACTIVE:
    {
      position = 0;
      state = AVRCP_PLAY_STATUS_ERROR;
      artist = "";
      if (hfp_connected())
      {
        avrcp_connect(*hfp_remote_addr());
        title = "Connecting";
      }
      window_invalid(NULL);

      uint8_t flag = window_readconfig()->gesture_flag;
      if (flag & BIT0)
      {
        // gesture is enabled
        gesture_init(0, flag & BIT1);
      }

      break;
    }
  case PROCESS_EVENT_TIMER:
    {
      if (position < length && state == AVRCP_PLAY_STATUS_PLAYING)
      {
        position++;
        window_timer(CLOCK_SECOND);
        window_invalid(&rect);
      }
      break;
    }
  case EVENT_NOTIFY_RESULT:
    window_close();
    break;
  case EVENT_AV:
    handle_av_events(lparam, rparam);
    break;
  case EVENT_WINDOW_PAINT:
    {
      OnDraw((tContext*)rparam);
      break;
    }
  case EVENT_GESTURE_MATCHED:
    if (lparam > 0)
    {
      uint8_t data = window_readconfig()->gesture_map[lparam];

      if (data & BIT0)
      {
        // Next Song
        lparam = KEY_DOWN;
      }
      else if (data & BIT1)
      {
          // Previous Song
          lparam = KEY_ENTER;
      }
      else if (data & BIT2)
      {
          //Play/Pause
          lparam = KEY_UP;
      }
      else if (data & BIT3)
      {
          // Volume Up
          avctp_send_passthrough(VOL_UP_OP);
          break;
      }
      else if (data & BIT4)
      {
          // Volume Down
          avctp_send_passthrough(VOL_DOWN_OP);
          break;
      }
    }
    else
      break;
    // fall through
  case EVENT_KEY_PRESSED:
    {
      if (state == AVRCP_PLAY_STATUS_ERROR)
        break;

      if (lparam == KEY_UP)
      {
        printf("MC State=%d\n", state);
        if (state == AVRCP_PLAY_STATUS_PLAYING)
        {
          avctp_send_passthrough(PAUSE_OP);
        }
        else
        {
          avctp_send_passthrough(PLAY_OP);
        }
      }
      else if (lparam == KEY_DOWN)
      {
        avctp_send_passthrough(FORWARD_OP);
      }
      else if (lparam == KEY_ENTER)
      {
        avctp_send_passthrough(BACKWARD_OP);
      }
      else if (lparam == KEY_TAP)
      {
        uint8_t flag = window_readconfig()->gesture_flag;
        if (flag & BIT0)
        {
          // gesture is enabled
          gesture_init(0, flag & BIT1);
        }
      }
      break;
    }
  case EVENT_WINDOW_CLOSING:
    {
      window_timer(0);
      break;
    }
  default:
    return 0;
  }
  return 1;
}
