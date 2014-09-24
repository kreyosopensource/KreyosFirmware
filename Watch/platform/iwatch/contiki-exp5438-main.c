/*
* Copyright (c) 2006, Swedish Institute of Computer Science
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the Institute nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* @(#)$Id: contiki-z1-main.c,v 1.4 2010/08/26 22:08:11 nifi Exp $
*/

#include "contiki.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dev/flash.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/watchdog.h"
#include "dev/xmem.h"
#include "lib/random.h"
#include "sys/profile.h"
#include "sys/ctimer.h"

#include "node-id.h"
#include "power.h"
#include "battery.h"
#include "button.h"
#include "spiflash.h"
#include "bluetooth.h"
#include "backlight.h"
#include "window.h"
#include "system.h"
#include "codec.h"

#include "ant/ant.h"
#include "ant/antinterface.h"

#include "system.h"
/*--------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// Function prototypes
extern int CheckUpgrade(void);
extern void Upgrade(void);
extern void uart_init(char rate);
extern void mpu6050_init();
extern void button_init();
extern void I2C_Init();
extern void rtc_init();
extern void battery_init();
extern unsigned char* CFSFontWrapperLoad();

static uint8_t msp430_dco_required = 0;

uint8_t shutdown_mode = 0;

/*--------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  /*
  * Initalize hardware.
  */
  msp430_cpu_init();
  clock_init();

  uart_init(9600); /* Must come before first printf */

  /* xmem_init(); */

  PRINTF("iWatch 0.10 build at " __TIME__ " " __DATE__ "\n");
  UCSCTL8 &= ~BIT2;
  
  /*
  * Hardware initialization done!
  */

  /*
  * Initialize Contiki and our processes.
  */
  process_init();
  process_start(&etimer_process, NULL);
  
  rtimer_init();
  ctimer_init();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  backlight_init();
  battery_init();
  SPI_FLASH_Init();

  if (system_testing())
  {
    clock_time_t t;

    backlight_on(200, 0);
    t = clock_seconds();
    // sleep 1
    while(clock_seconds() - t <= 3);
    printf("$$OK BACKLIGHT\n");
    t = clock_seconds();
    while(clock_seconds() - t <= 3);
    backlight_on(0, 0);

    motor_on(200, 0);
    // sleep 1s
    t = clock_seconds();
    while(clock_seconds() - t <= 3);
    printf("$$OK MOTOR\n");
    t = clock_seconds();
    while(clock_seconds() - t <= 3);
    motor_on(0, 0);

#if PRODUCT_W001
    I2C_Init();
    codec_init();
    codec_bypass(1);
    // sleep 1s
    t = clock_seconds();
    while(clock_seconds() - t <= 3);
    printf("$$OK MIC\n");
    // sleep 1s
    t = clock_seconds();
    while(clock_seconds() - t <= 3);
    codec_bypass(0);

    codec_shutdown();
#endif
  }

  int reason = CheckUpgrade();

  window_init(reason);

  button_init();
  rtc_init();
  CFSFontWrapperLoad();

  system_init(); // check system status and do factor reset if needed

  I2C_Init();

  //codec_init();
  //ant_init();
  bluetooth_init();

#ifdef PRODUCT_W004
  //bmx_init();
#else
  mpu6050_init();
#endif

  // check the button status
  if (button_snapshot() & (1 << BUTTON_UP))
  {
    clock_time_t t;
    // delay 1 second
    // button up is pressed, we will set emerging flag
    motor_on(200, CLOCK_SECOND * 2);
    t = clock_seconds();
    while(clock_seconds() - t <= 1);

    if (button_snapshot() & (1 << BUTTON_UP)) 

    system_setemerging();
    motor_on(0, 0);
  }  
  
  if (!system_retail())
  {
    bluetooth_discoverable(1);
  }

#if PRODUCT_W001
  if (system_testing())
    ant_init(MODE_HRM);
#endif
  
  system_restore();

//  protocol_init();
//  protocol_start(1);
  
  process_start(&system_process, NULL);

  /*
  * This is the scheduler loop.
  */
  msp430_dco_required = 0;

  /*
    check firmware update
    */
  if (reason == 0xff)
  {
    printf("Start Upgrade\n");
    Upgrade();
    // never return if sucessfully upgrade
  }

  watchdog_start();

  while(1) {
    int r;
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);

    /*
    * Idle processing.
    */
    int s = splhigh();          /* Disable interrupts. */
    /* uart1_active is for avoiding LPM3 when still sending or receiving */
    if(process_nevents() != 0) {
      splx(s);                  /* Re-enable interrupts. */
    } else {
      static unsigned long irq_energest = 0;

      /* Re-enable interrupts and go to sleep atomically. */
      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);
      /* We only want to measure the processing done in IRQs when we
         are asleep, so we discard the processing time done when we
         were awake. */
      energest_type_set(ENERGEST_TYPE_IRQ, irq_energest);
      watchdog_stop();

      if (shutdown_mode)
      {
        system_shutdown(1); // never return
        LPM4;
      }
      
      if (msp430_dco_required)
      {
        __low_power_mode_0();
      }
      else
      {
        __low_power_mode_3();
      }

      /* We get the current processing time for interrupts that was
         done during the LPM and store it for next time around.  */
      __disable_interrupt();
      irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
      __enable_interrupt();
      watchdog_start();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
    }
  }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* add/remove_lpm_req - for requiring a specific LPM mode. currently Contiki */
/* jumps to LPM3 to save power, but DMA will not work if DCO is not clocked  */
/* so some modules might need to enter their LPM requirements                */
/* NOTE: currently only works with LPM1 (e.g. DCO) requirements.             */
/*---------------------------------------------------------------------------*/
void
power_pin(uint8_t module)
{
    msp430_dco_required |= module;
}

void
power_unpin(uint8_t module)
{
    msp430_dco_required &= ~module;
}