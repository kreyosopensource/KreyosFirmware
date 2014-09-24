
#include "contiki.h"
#include "sys/etimer.h"

PROCESS(bluetooth_process, "Bluetooth process");

#include <stdio.h> /* For log_info() */
#include <string.h>

#include <btstack/run_loop.h>

#include "hci.h"

static void callback(void *ptr)
{
  timer_source_t *timer = (timer_source_t *)ptr;
  timer->process(ptr);
}

// set timer based on current time
void run_loop_set_timer(timer_source_t *a, uint32_t timeout_in_ms)
{
  clock_time_t ticks = embedded_ticks_for_ms(timeout_in_ms);

  a->_timer.ptr = a;
  a->timeout = ticks;
}

void run_loop_set_timer_handler(timer_source_t *ts, void (*process)(timer_source_t *_ts))
{
  ts->process = process;
}

// add/remove timer_source
void run_loop_add_timer(timer_source_t *a)
{
  ctimer_set(&a->_timer, a->timeout, callback, a->_timer.ptr);
}

int  run_loop_remove_timer(timer_source_t *a)
{
  ctimer_stop(&a->_timer);
  return 1;
}

// add/remove data_source
static linked_list_t data_sources;

void run_loop_add_data_source(data_source_t *ds)
{
  linked_list_add(&data_sources, (linked_item_t *) ds);
}

int  run_loop_remove_data_source(data_source_t *ds)
{
  return linked_list_remove(&data_sources, (linked_item_t *) ds);
}

PROCESS_THREAD(bluetooth_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  // wait RTS of BT pull down to 0
  //BUSYWAIT_UNTIL(0, RTIMER_SECOND);
  BUSYWAIT_UNTIL(((BT_CTS_IN & BT_CTS_BIT)==0), RTIMER_SECOND/10);
  if((BT_CTS_IN & BT_CTS_BIT)==0)
  {
    printf("CTS is ready");
  }


  // wait about 100ms for bluetooth to start
  etimer_set(&timer, CLOCK_SECOND/10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

  // turn on!
  hci_power_control(HCI_POWER_ON);

  while(1)
  {
    PROCESS_WAIT_EVENT();

    if (ev != PROCESS_EVENT_POLL)
      continue;

    // process data sources
    data_source_t *next;
    data_source_t *ds;
    for (ds = (data_source_t *) data_sources; ds != NULL ; ds = next){
      next = (data_source_t *) ds->item.next; // cache pointer to next data_source to allow data source to remove itself
      ds->process(ds);
    }
  }

  PROCESS_END();
}
