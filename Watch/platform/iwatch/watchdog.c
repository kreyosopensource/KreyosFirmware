/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: watchdog.c,v 1.12 2010/11/12 15:54:41 nifi Exp $
 */

#include "contiki.h"
#include "dev/watchdog.h"
#include "isr_compat.h"
#include <stdio.h>
#include <string.h>
#include "button.h"
#include "grlib/grlib.h"
#include "sys/process.h"
#include "system.h"
#include "backlight.h"

extern void flushlcdsync();
extern void rtc_save();

static int counter = 0;

#define PRINT_STACK_ON_REBOOT 0

#if PRINT_STACK_ON_REBOOT == 1
static void printstack(uint16_t *ptr)
{
  int i;

  printf("Watchdog reset");
  printf("\nStack at 0x%p:\n", ptr);

  for(i = 0; i < 32; ++i) {
    printf("%x", ptr[i]);
    putchar(' ');
    if((i & 0x0f) == 0x0f) {
      putchar('\n');
    }
  }
  putchar('\n');
}
#endif

extern tContext context;
extern const tRectangle fullscreen_clip;

static void displaystack(uint16_t *ptr)
{
  char buf[200] = "Crash Info\n";
  
  for(int i = 0; i < 16; ++i)
  {
    char buf0[10];
    sprintf(buf0, "%04x ", ptr[i]);
    strcat(buf, buf0);
  }
  
  GrContextClipRegionSet(&context, &fullscreen_clip);
  GrContextForegroundSet(&context, ClrBlack);
  GrRectFill(&context, &fullscreen_clip);

  GrContextForegroundSet(&context, ClrWhite);
  GrContextFontSet(&context, (tFont*)&g_sFontGothic14);
  GrStringDrawWrap(&context, buf, 2, 16, 120, 0);
  
  GrStringDraw(&context, PROCESS_CURRENT()->name, -1, 2, 100, 0);
  //GrFlush(&context);
  flushlcdsync();
  
  while(!(button_snapshot() & (1 << BUTTON_UP)));
}

/*---------------------------------------------------------------------------*/
ISR(WDT, watchdog_interrupt)
{
  uint16_t dummy;

  motor_on(0, 0);
#if PRINT_STACK_ON_REBOOT == 1
  printstack(&dummy);
#endif
#if PRINT_STACK_ON_REBOOT == 2
  displaystack(&dummy);
#endif /* PRINT_STACK_ON_REBOOT */
  system_reset();
}
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  /* The MSP430 watchdog is enabled at boot-up, so we stop it during
     initialization. */
  counter = 0;
  watchdog_stop();

  SFRIFG1 &= ~WDTIFG;
  SFRIE1 |= WDTIE;
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  /* We setup the watchdog to reset the device after one second,
     unless watchdog_periodic() is called. */
  counter--;
  if(counter == 0) {
    WDTCTL = WDTPW | WDTCNTCL | WDT_ARST_1000 | WDTTMSEL;
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  /* This function is called periodically to restart the watchdog
     timer. */
  /*  if(counter < 0) {*/
    WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL | WDTTMSEL;
    /*  }*/
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  counter++;
  if(counter == 1) {
    WDTCTL = WDTPW | WDTHOLD;
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  WDTCTL = 0;
}
/*---------------------------------------------------------------------------*/
