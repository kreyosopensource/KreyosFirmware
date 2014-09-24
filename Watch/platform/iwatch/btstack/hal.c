
#include "contiki.h"
#include "bluetooth.h"

#include "sys/clock.h"

//extern uint8_t bluetooth_uart_active;

void hal_cpu_set_uart_needed_during_sleep(uint8_t enabled)
{
  //bluetooth_uart_active = enabled;
}

uint32_t embedded_get_ticks(void)
{
  return clock_time();
}

uint32_t embedded_ticks_for_ms(uint32_t time_in_ms)
{
  return time_in_ms * CLOCK_SECOND / 1000;
}

