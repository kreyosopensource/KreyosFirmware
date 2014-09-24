#include "contiki.h"
#include "window.h"
#include "grlib/grlib.h"
#include "system.h"
#include "icons.h"

uint8_t reset_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
    case EVENT_WINDOW_CREATED:
      window_messagebox(ICON_LARGE_WARNING, "Reset watch to factory settings?", NOTIFY_CONFIRM);
      return 0;

    case EVENT_NOTIFY_RESULT:
      if (lparam == 1)
      {
      	system_resetfactory();
      }
      window_close();
      break;
  }

  return 0;
}
