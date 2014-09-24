/*
* This file implment the notification for the incoming phone
*/

#include "contiki.h"
#include "window.h"
#include "backlight.h"
#include "hfp.h"
#include "bluetooth.h"
#include "icons.h"
#include <string.h>
#include <stdio.h>
#include "stlv_client.h"
#include "stlv_handler.h"

static void onDraw(tContext *pContext)
{
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &client_clip);
  GrContextForegroundSet(pContext, ClrWhite);
  
  GrContextFontSet(pContext, &g_sFontGothic18);
  GrStringDrawCentered(pContext, "Voice Command", -1, 72, 80, 0);
  window_button(pContext, KEY_EXIT, "Finish");

  // volume
  window_button(pContext, KEY_ENTER, "Again");
  window_button(pContext, KEY_UP, "Vol Up");
  window_button(pContext, KEY_DOWN, "Vol Down");
  window_volume(pContext, 30, 125, 8, codec_getvolume());
}

uint8_t siri_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  case EVENT_WINDOW_CREATED:
    if (!hfp_connected())
    {
        window_messagebox(ICON_LARGE_WARNING, PairingWarning, 0);
        return 1;
    }
    
    if (rparam)
    {
      if (get_phone_type() == PHONE_TYPE_ANDROID)
        launch_google_now();
      else
        hfp_enable_voicerecog(1);
    }

    codec_setvolume(window_readconfig()->volume_level);
    break;
  case EVENT_BT_BVRA:
    if (lparam == 0)
    {
      window_close();
    }
    break;
  case EVENT_KEY_PRESSED:
  switch(lparam)
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
        case KEY_ENTER:
        {
          if (get_phone_type() == PHONE_TYPE_ANDROID)
            launch_google_now();
          else
            hfp_enable_voicerecog(1);
          break;
        }
      }
      window_invalid(NULL);
    break;
  case EVENT_WINDOW_PAINT:
    onDraw((tContext*)rparam);
    break;
  case EVENT_NOTIFY_RESULT:
    window_close();
    break;
  case EVENT_EXIT_PRESSED:
    {
      int level = window_readconfig()->volume_level;
      if (level != codec_getvolume())
      {
        window_readconfig()->volume_level = codec_getvolume();
        window_writeconfig();
      }
      
      hfp_enable_voicerecog(0);
      window_close();
    
    break;
    }
  default:
    return 0;
  }
  
  return 1;
}
