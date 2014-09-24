#include "contiki.h"
#include "window.h"
#include "memlcd.h"
#include "ant/ant.h"
#include "stlv_client.h"
#include "ble_handler.h"

static uint8_t selection;
static uint8_t sports_type = 0;

static void onDraw(tContext *pContext)
{
  GrContextForegroundSet(pContext, ClrBlack);
  GrRectFill(pContext, &fullscreen_clip);

  GrContextForegroundSet(pContext, ClrWhite);

  // display the text
#if PRODUCT_W001
  GrStringDraw(pContext,"Wait for GPS/ANT", -1, 10, 80, 0);
#else
  GrStringDraw(pContext,"Wait for GPS", -1, 10, 80, 0);
#endif
  if (sports_type == SPORTS_DATA_FLAG_RUN)
    window_button(pContext, KEY_ENTER, " IGNORE");
}

uint8_t sportwait_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev){
  case EVENT_WINDOW_CREATED:
      selection = (uint8_t)rparam;
      if (selection == 0)
      {
        //running
        sports_type = SPORTS_DATA_FLAG_RUN;
#if PRODUCT_W001
        ant_init(MODE_HRM);
#endif
      }
      else
      {
        //cycling
        sports_type = SPORTS_DATA_FLAG_BIKE;
#if PRODUCT_W001
        ant_init(MODE_CBSC);
#endif
      }

      //send out start workout data to watch
      {
        uint8_t stlv_meta = 0;
        uint32_t stlv_data = 0;

        //STLV over RFCOMM
        send_sports_data(0, sports_type | SPORTS_DATA_FLAG_PRE, &stlv_meta, &stlv_data, 1);

        //BLE
        ble_start_sync(1, 0);
      }
    return 0x80;
  case EVENT_SPORT_DATA:
    window_close(); // close self
    window_open(&sportswatch_process, (void*)selection);
    break;
  case EVENT_WINDOW_CLOSING:
    {
      uint8_t stlv_meta = 0;
      uint32_t stlv_data = 0;
      send_sports_data(0, sports_type | SPORTS_DATA_FLAG_STOP, &stlv_meta, &stlv_data, 1);

      ble_stop_sync();
    }
    break;
  case EVENT_WINDOW_PAINT:
    onDraw((tContext*)rparam);
    break;
  case EVENT_NOTIFY_RESULT:
    window_close();
    break;
  case EVENT_KEY_PRESSED:
    if (lparam == KEY_ENTER)
    {
      window_close(); // close self
      window_open(&sportswatch_process, (void*)selection);
    }
  default:
    return 0;
  }

  return 1;
}
