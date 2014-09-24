#include "contiki.h"
#include "window.h"
#include "gesture/gesture.h"
#include "battery.h"
#include "hfp.h"
#include "backlight.h"
#include <stdio.h>
#include <string.h>

#include "cfs/cfs.h"

static enum {NONE, RECORDING, RECOGNIZE, DONE} state;
static uint8_t matched = 0;

void test_cfs()
{
  /*        */
  /* step 1 */
  /*        */
  char message[32];
  char buf[100];
  strcpy(message,"#1.hello world.");
  strcpy(buf,message);
  printf("step 1: %s\n", buf );
  /* End Step 1. We will add more code below this comment later */    
  /*        */
  /* step 2 */
  /*        */
  /* writing to cfs */
  char *filename = "msg_file";
  int fd_write, fd_read;
  int n;
  fd_write = cfs_open(filename, CFS_WRITE);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    printf("step 2: successfully written to cfs. wrote %i bytes\n", n);
  } else {
    printf("ERROR: could not write to memory in step 2.\n");
  } 
  /*        */
  /* step 3 */
  /*        */
  /* reading from cfs */
  strcpy(buf,"empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read!=-1) {
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 3: %s\n", buf);
    cfs_close(fd_read);
  } else {
    printf("ERROR: could not read from memory in step 3.\n");
  }
  /*        */
  /* step 4 */
  /*        */
  /* adding more data to cfs */
  strcpy(buf,"empty string");
  strcpy(message,"#2.contiki is amazing!");
  fd_write = cfs_open(filename, CFS_WRITE | CFS_APPEND);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    printf("step 4: successfully appended data to cfs. wrote %i bytes  \n",n);
  } else {
    printf("ERROR: could not write to memory in step 4.\n");
  }
  /*        */
  /* step 5 */
  /*        */
  /* seeking specific data from cfs */
  strcpy(buf,"empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read != -1) {
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 5: #1 - %s\n", buf);
    cfs_seek(fd_read, sizeof(message), CFS_SEEK_SET);
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 5: #2 - %s\n", buf);
    cfs_close(fd_read);
  } else {
    printf("ERROR: could not read from memory in step 5.\n");
  }
  /*        */
  /* step 6 */
  /*        */
  /* remove the file from cfs */
  cfs_remove(filename);
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read == -1) {
    printf("Successfully removed file\n");
  } else {
    printf("ERROR: could read from memory in step 6.\n");
  }
  
}
void handle_message(uint8_t msg_type, char* ident, char* message);

uint8_t selftest_process(uint8_t ev, uint16_t lparam, void* rparam)
{
  switch(ev)
  {
  case EVENT_WINDOW_CREATED:
    {
      //test_cfs();
      window_timer(CLOCK_SECOND * 5);
      break;
    }
  case EVENT_WINDOW_PAINT:
    {
      tContext *pContext = (tContext*)rparam;
      GrContextForegroundSet(pContext, ClrBlack);
      GrRectFill(pContext, &client_clip);
      GrContextForegroundSet(pContext, ClrWhite);
      
      GrContextFontSet(pContext, &g_sFontGothic18);
      const char* msg;
      BATTERY_STATE batterystate = battery_state();
      // draw the state
      switch(batterystate)
      {
      case BATTERY_STATE_DISCHARGING:
        msg = "battery is discharging.";
        break;
      case BATTERY_STATE_CHARGING:
        msg = "battery is charging.";
        break;
      case BATTERY_STATE_FULL:
        msg = "battery is full. charged stopped.";
        break;
      }
      GrStringDraw(pContext, msg, -1, 10, 20, 0);
      char buf[50];
      uint8_t level = battery_level(batterystate);
      sprintf(buf, "battery level is %d\n", level);
      GrStringDraw(pContext, buf, -1, 10, 40, 0);
      
#if ENERGEST_CONF_ON
      sprintf(buf, "cpu %lu lpm %lu irq %lu serial %lu\n",
              energest_total_time[ENERGEST_TYPE_CPU].current,
              energest_total_time[ENERGEST_TYPE_LPM].current,
              energest_total_time[ENERGEST_TYPE_IRQ].current,
              energest_total_time[ENERGEST_TYPE_SERIAL].current);
      
      printf("cpu %lu lpm %lu irq %lu serial %lu\n",
             energest_total_time[ENERGEST_TYPE_CPU].current,
             energest_total_time[ENERGEST_TYPE_LPM].current,
             energest_total_time[ENERGEST_TYPE_IRQ].current,
             energest_total_time[ENERGEST_TYPE_SERIAL].current);
      
      GrStringDrawWrap(pContext, buf, 0, 55, 100, 1);
#endif
      
      if (state == RECORDING)
      {
        GrStringDraw(pContext, "RECORDING", -1, 15, 60, 0);	
      }
      else if (state == RECOGNIZE)
      {
        if (matched != 0)
        {
          char buf[30];
          sprintf(buf, "matched %d", matched);
          GrStringDraw(pContext, buf, -1, 15, 90, 0);
        }
        GrStringDraw(pContext, "RECOGNIZE", -1, 15, 60, 0);
      }
      
      window_timer(CLOCK_SECOND * 5);
      break;
    }
  case EVENT_GESTURE_MATCHED:
    {
      matched = lparam;
      window_invalid(NULL);
      if (matched != 0)
        motor_on(200, CLOCK_SECOND/4);
      break;
    }
  case PROCESS_EVENT_TIMER:
    window_invalid(NULL);
    break;
  case EVENT_KEY_PRESSED:
    if (lparam == KEY_ENTER)
    {
      for(int i = 0; i < 10; i++)
      {
        char name[32];
        sprintf(name, "record%d.dat", i);
        
        int fd = cfs_open(name, CFS_READ);
        if (fd == -1)
          break;
        uint8_t buf[128];
        int len = cfs_read(fd, buf, 128);
        while(len > 0)
        {
          hexdump(buf, len);
          len = cfs_read(fd, buf, 128);
        }
        cfs_close(fd);
      }
      
    }
#if 1
    //	hfp_enable_voicerecog();
    else if (lparam == KEY_UP)
    {
      //handle_message('S', "From: +8615618273349", "Message: Shd/dhdjbdjhdbd#shs#bdhjsbxhxjjxhdhdhhdjjdjd");
      printf("\nStart Recoding...\n");
      gesture_init(1, 0); // recording
      state = RECORDING;
    }
    else if (lparam == KEY_DOWN)
    {
      printf("\nStart Recongize...\n");
      gesture_init(0, 0); // recording	
      state = RECOGNIZE;
      matched = 0;
    }
  #else
    else if (lparam == KEY_UP)
    {
      static const uint8_t ancsuuid[] = {
        0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79
      };
      //printf("att_server_query_service\n");
      //att_server_query_service(ancsuuid);
      att_server_send_gatt_services_request();
    }
    #endif
    window_invalid(NULL);
    break;
  case EVENT_WINDOW_CLOSING:
    window_timer(0);
    break;
  default:
    return 0;
  }
  
  return 1;
}
