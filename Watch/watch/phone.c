/*
* This file implment the notification for the incoming phone
*/

#include "contiki.h"
#include "window.h"
#include "backlight.h"
#include "hfp.h"
#include "gesture/gesture.h"
#include "bluetooth.h"
#include <string.h>
#include <stdio.h>

static char phonenumber[20];

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

/*
* The dialog shows the option to accept the call, reject call
* or send SMS (if we can get MNS works)
* After that, give the option to hang up
* The dialog will be show as an notification when callsetup = 1
*/
static void onDraw(tContext *pContext)
{
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &client_clip);
  GrContextForegroundSet(pContext, ClrWhite);
  
  GrContextFontSet(pContext, &g_sFontGothic18);
  if (hfp_getstatus(HFP_CIND_CALL) == HFP_CIND_CALL_ACTIVE)
  {
    GrStringDrawCentered(pContext, "Calling", -1, 72, 60, 0);
    window_button(pContext, KEY_EXIT, "Hang up");

    // volume
    window_button(pContext, KEY_UP, "Vol Up");
    window_button(pContext, KEY_DOWN, "Vol Down");
    window_volume(pContext, 30, 125, 8, codec_getvolume());
  }
  else if (hfp_getstatus(HFP_CIND_CALLSETUP) == HFP_CIND_CALLSETUP_INCOMING)
  {
      GrStringDrawCentered(pContext, "Incoming Call", -1, 72, 60, 0);
      window_button(pContext, KEY_EXIT, "Decline");
      window_button(pContext, KEY_ENTER, "Answer");
  }
  else if (hfp_getstatus(HFP_CIND_CALLSETUP) == HFP_CIND_CALLSETUP_OUTGOING)
  {
      GrStringDrawCentered(pContext, "Outgoing Call", -1, 72, 60, 0);
      window_button(pContext, KEY_EXIT, "Finish");
  }
  else if (hfp_getstatus(HFP_CIND_CALLSETUP) == HFP_CIND_CALLSETUP_ALERTING)
  {
      GrStringDrawCentered(pContext, "Outgoing Call", -1, 72, 60, 0);
      window_button(pContext, KEY_EXIT, "Finish");
  }
  else
  {
    GrStringDrawCentered(pContext, "Call is finished", -1, 72, 60, 0);	
    window_close();
    
    return; // don't need paint others
  }

  // draw the phone number
  GrContextFontSet(pContext, get_titlefont());
  GrStringDrawCentered(pContext, phonenumber, -1, 72, 80, 0);
}

static void handleKey(uint8_t key)
{
  switch(hfp_getstatus(HFP_CIND_CALL))
  {
  case HFP_CIND_CALL_NONE:
    /* ring, down/enter-> pick, up -> SMS, exit -> reject */
    switch(key)
    {
    case KEY_DOWN:
    case KEY_ENTER:
      // notify hfp that we are accepting the call
      hfp_accept_call(1);
      break;
    }
    break;
    
    // exit, hang up the call
  case HFP_CIND_CALL_ACTIVE:
    {
      switch(key)
      {
      case KEY_UP:
        {
          codec_changevolume(+1);
          break;
        }
      case KEY_DOWN:
        {
          // decrease voice
          codec_changevolume(-1);
          break;
        }
      }
    }
    
  }
  
  window_invalid(NULL);
}

uint8_t phone_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  case EVENT_WINDOW_CREATED:
    {
      if (!hfp_connected())
        window_close();
      if (rparam == NULL)
        phonenumber[0] = '\0';
      else
        strcpy(phonenumber, rparam);

      uint8_t flag = window_readconfig()->gesture_flag;
      if (flag & BIT0)
      {
        // gesture is enabled
        gesture_init(0, flag & BIT1);
      }
      codec_setvolume(window_readconfig()->volume_level);
    }
    break;
  case EVENT_BT_CLIP:
    if (phonenumber[0] == '\0')
    {
      strcpy(phonenumber, rparam);
      window_invalid(NULL);			
    }
    break;
  case EVENT_BT_CIEV:
    if ((lparam >> 8) == HFP_CIND_CALL)
    {
      if ((lparam & 0x0f) == 1)
      {
        motor_on(100, CLOCK_SECOND/2);
      }
    }
    
    window_invalid(NULL);
    break;
  case EVENT_BT_RING:
    motor_on(100, CLOCK_SECOND * 2 /3);
    break;

  case EVENT_GESTURE_MATCHED:
    if (lparam > 0)
    {
      uint8_t data = window_readconfig()->gesture_map[lparam];

      if (data & BIT5)
      {
          handleKey(KEY_ENTER);
      }
      else if (data & BIT6)
      {
          handleKey(KEY_EXIT);
      }
    }
    break;
  case EVENT_WINDOW_PAINT:
    onDraw((tContext*)rparam);
    break;
    
  case EVENT_KEY_PRESSED:
    handleKey((uint8_t)lparam);
    break;
    
  case EVENT_EXIT_PRESSED:
    {
      int level = window_readconfig()->volume_level;
      if (level != codec_getvolume())
      {
        window_readconfig()->volume_level = codec_getvolume();
        window_writeconfig();
      }

      hfp_accept_call(0);
      gesture_shutdown();
      window_close();
      break;
    }
  default:
    return 0;
  }
  
  return 1;
}
