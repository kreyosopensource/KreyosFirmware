
#ifdef __WIN32 
#include <windows.h>
#endif

#include "contiki.h"
#include "rtc.h"
#include "battery.h"
#include "ant/ant.h"
#include <stdio.h>
#include <string.h>
#include "window.h"
#include "btstack/utils.h"

uint8_t shutdown_mode;

#define NOT_IMPL_VOID(f, ...) void f(__VAR_ARGS){printf("[Not Impl]%s\n", __func__);}
#define NOT_IMPL(f, rettype, ret, ...) rettype f(__VAR_ARGS){printf("[Not Impl]%s\n", __func__);return ret;}

void rtc_init(){}
extern void rtc_setdate(uint16_t year, uint8_t month, uint8_t day)
{
}
void rtc_settime(uint8_t hour, uint8_t min, uint8_t sec)
{
}

void rtc_setalarm(uint8_t aday, uint8_t adow, uint8_t ahour, uint8_t aminute)
{

}

void rtc_readtime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
  if (hour) *hour = 17;
  if (min) *min = 24;
  if (sec) *sec = 47;
}
void rtc_readdate(uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *weekday)
{
  if (year) *year = 2014;
  if (month) *month = 1;
  if (day) *day = 1;
  if (weekday) *weekday = 3;
}

uint8_t rtc_getmaxday(uint16_t year, uint8_t month)
{
  if (month == 2)
    {
      if (year%4==0 && year%100==0 || year%400==0)
	return 29;
      else
	return 28;
    }
  if(month==8)
    {
      return 31;
    }

  if (month % 2 == 0)
    {
      return 30;
    }
  else
    {
      return 31;
    }
}

uint8_t rtc_getweekday(uint16_t year, uint8_t month, uint8_t day)
{
	  if( month == 1 || month == 2 )
  {
    month += 12;
    year--;
  }

  return 1 + (( day + 2*month + 3*(month+1)/5 + year + year/4 ) %7);
}

void rtc_enablechange(uint8_t changes)
{

}

void watchdog_stop() {}
void watchdog_start() {}

void I2C_Init() {}
void button_init() {}
void backlight_init() {}
void rtimer_arch_init(void) {}
void rtimer_arch_schedule(rtimer_clock_t t){}
uint8_t battery_level(BATTERY_STATE state) {return 7;}
BATTERY_STATE battery_state() {return BATTERY_STATE_CHARGING;}
void backlight_on(uint8_t level, clock_time_t length) {}
void mpu6050_init() {}


uint8_t selftest_process(uint8_t ev, uint16_t lparam, void* rparam) {return 0;}

void flash_setup(void) {}
void flash_done(void) {}
void flash_write(uint16_t *addr, unsigned short word) {}
void flash_clear(uint16_t *addr) {}

void flash_writepage(uint16_t *addr, const uint16_t *data, uint8_t size) {}
void __disable_interrupt() {}

unsigned long mpu_getsteptime()
{return 1257;}
unsigned long mpu_getsteps()
{return 1435;}
void mpu_switchmode(int d)
{}

void ant_init(ModeEnum mode) {}
void ant_shutdown(void) {}

#ifdef _WINDOWS_H
void nanosleep(int millisecond)
{
    Sleep(millisecond);
}
#endif

int spp_send(char* buf, int count)
{
  return 0;
}

void motor_on(uint8_t level, clock_time_t length)
{

}

//int spp_register_task(char* buf, int size, void (*callback)(char*, int))
//{}

void system_ready()
{
  printf("system is ready\n");
}

uint8_t system_testing()
{
  return 0;
}

void system_rebootToNormal()
{

}

int mpu6050_selftest()
{
  return 0;
}

static char fake[6] = {43,44,45,86,97,32};
 uint8_t * system_getserial()
 {
  return fake;
 }


uint8_t
ANT_ChannelPower(
   uint8_t ucANTChannel_, 
   uint8_t ucPower_)
{
  return 0;
}



FILE *fp;
void SPI_FLASH_BufferRead_Raw(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  if (fp == NULL)
  {
    fp = fopen("fontunicode18pt.bin", "rb");
  }

  printf("Read %x\n", ReadAddr);
  fseek(fp, ReadAddr, SEEK_SET);
  fread(pBuffer,   NumByteToRead, 1, fp);
}

void bluetooth_enableConTxMode(int mode, int freq)
{
}

uint16_t ped_get_steps()
{
  return 6000;
}

uint16_t ped_get_calorie() {return 10;}
uint16_t ped_get_time() {return 10;}
uint16_t ped_get_distance(){return 10;}

void ped_reset(){}


uint16_t __swap_bytes(uint16_t d)
{
  
  uint8_t *p = (uint8_t*)&d;
  uint8_t tmp = p[1];
  p[1] = p[0];
  p[0] = tmp;

  return d;
}

void codec_setvolume(int a) {}

const char buf[] = "THISISAD";
const char *bluetooth_address() {
  return &buf[0];
}


static const uint8_t month_day_map[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};
uint32_t calc_timestamp(uint8_t year, uint8_t month, uint8_t day, uint8_t hh, uint8_t mm, uint8_t ss)
{
    uint8_t leap_years = year / 4 + 1;
    if (year % 4 == 0 && month < 2)
        leap_years -= 1;

    uint32_t days = year * 365 + leap_years;
    for (uint8_t i = 0; i < month - 1; ++i)
        days += month_day_map[i];
    days += day;
    return ((days * 24 + hh) * 60 + mm) * 60 + ss;

}

void parse_timestamp(uint32_t time, uint8_t* year, uint8_t* month, uint8_t* day, uint8_t* hh, uint8_t* mm, uint8_t* ss)
{
    uint32_t temp = 0;

    *ss = time % 60;
    temp = time / 60;

    *mm = temp % 60;
    temp = temp / 60;

    *hh = temp % 24;
    temp = temp / 24;

    uint16_t total_day = temp;
    uint8_t tyear = 0;
    while(1)
    {
        if (tyear % 4 == 0)
        {
            if (total_day >= 366)
                total_day -= 366;
            else
                break;
        }
        else
        {
            if (total_day >= 365)
                total_day -= 365;
            else
                break;
        }
        tyear++;
    }

    uint8_t i = 0;
    for (; i < count_elem(month_day_map); ++i)
    {
        if (tyear % 4 == 0 && i == 1)
        {
            if (total_day < month_day_map[i] + 1)
                break;
            total_day -= month_day_map[i] + 1;
        }
        else
        {
            if (total_day < month_day_map[i])
                break;
            total_day -= month_day_map[i];
        }
    }

    *year  = tyear;
    *month = i + 1;
    *day   = total_day;

}

uint32_t rtc_readtime32()
{
    uint16_t year  = 0;
    uint8_t  month = 0;
    uint8_t  day   = 0;
    uint8_t  wday  = 0;
    rtc_readdate(&year, &month, &day, &wday);

    uint8_t  hour  = 0;
    uint8_t  min   = 0;
    uint8_t  sec   = 0;
    rtc_readtime(&hour, &min, &sec);
    return calc_timestamp(year - 2000, month, day, hour, min, sec);
}

int avrcp_get_attributes(uint32_t item) {return 1;}
int avrcp_get_playstatus() {return 1;}
uint8_t avrcp_connected(){ return 0;}
void avctp_send_passthrough(uint8_t op) {}
void avrcp_connect(bd_addr_t remote_addr) {}
bd_addr_t* hfp_remote_addr() {return NULL;}

NOT_IMPL_VOID(codec_wakeup);
NOT_IMPL_VOID(codec_suspend);

#include "hci.h"
int hci_send_cmd(const hci_cmd_t *cmd, ...) {return 0;}

int hci_send_cmd_packet(uint8_t *packet, int size){return 0;}

NOT_IMPL_VOID(WriteFirmware, void* data, uint32_t offset, int size);
NOT_IMPL_VOID(EraseFirmware);
NOT_IMPL_VOID(Upgrade);
NOT_IMPL_VOID(system_reset);
NOT_IMPL(system_locked, uint8_t, 0);
NOT_IMPL(system_retail, uint8_t, 1);
NOT_IMPL_VOID(system_resetfactory);
NOT_IMPL(CheckUpgrade, int, 0);

NOT_IMPL_VOID(bluetooth_discoverable);
NOT_IMPL_VOID(bluetooth_init);
NOT_IMPL_VOID(bluetooth_shutdown);
NOT_IMPL(bluetooth_running, uint8_t, 1);

NOT_IMPL_VOID(spp_sniff, int onoff);
NOT_IMPL_VOID(system_unlock);

void hfp_battery(uint8_t level) {}

uint8_t hci_le_data_packet_length()
{
    return 26;
}
void att_fetch_next(uint32_t uid, uint32_t combine)
{
  switch(uid)
  {
    case 10:
    window_notify_content("mail", "from junsu", "message 0, 126312764128364817236417234612837463278467128346124128346", "20140302T110211", NOTIFY_OK, 'a');
    break;
    case 11:
    window_notify_content("mail", "from junsu", "message 1, hello fasudyreneu ds auwe", "20140302T110211", NOTIFY_OK, 'a');
    break;
    case 12:
    window_notify_content("mail", "from junsu", "message 2, dasdh dasfjds dasjhasdkjf  asjdhaksdf adfhkasfd kjdafsjk ajsdfasdf", "20140302T110211", NOTIFY_OK, 'a');
    break;
    default:
      window_notify_content("mail", "ERROR", "ERROR", "20140302T110211", NOTIFY_OK, 'a');
  }

}

void ble_advertise(uint8_t onoff)
{

}
void bluetooth_start(void)
{}
