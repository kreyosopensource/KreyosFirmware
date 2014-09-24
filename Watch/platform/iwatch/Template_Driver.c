/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//
// Template_Driver.c - Display driver for any LCD Controller. This file serves as
//						a template for creating new LCD driver files
//
// Copyright (c) 2008-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// READ ME
//
// This template driver is intended to be modified for creating new LCD drivers
// It is setup so that only Template_DriverPixelDraw() and DPYCOLORTRANSLATE()
// and some LCD size configuration settings in the header file Template_Driver.h
// are REQUIRED to be written. These functions are marked with the string
// "TemplateDisplayFix" in the comments so that a search through Template_Driver.c and
// Template_Driver.h can quickly identify the necessary areas of change.
//
// Template_DriverPixelDraw() is the base function to write to the LCD
// display. Functions like WriteData(), WriteCommand(), and SetAddress()
// are suggested to be used to help implement the Template_DriverPixelDraw()
// function, but are not required. SetAddress() should be used by other pixel
// level functions to help optimize them.
//
// This is not an optimized driver however and will significantly impact
// performance. It is highly recommended to first get the prototypes working
// with the single pixel writes, and then go back and optimize the driver.
// Please see application note www.ti.com/lit/pdf/slaa548 for more information
// on how to fully optimize LCD driver files. In short, driver optimizations
// should take advantage of the auto-incrementing of the LCD controller.
// This should be utilized so that a loop of WriteData() can be used instead
// of a loop of Template_DriverPixelDraw(). The pixel draw loop contains both a
// SetAddress() + WriteData() compared to WriteData() alone. This is a big time
// saver especially for the line draws and Template_DriverPixelDrawMultiple.
// More optimization can be done by reducing function calls by writing macros,
// eliminating unnecessary instructions, and of course taking advantage of other
// features offered by the LCD controller. With so many pixels on an LCD screen
// each instruction can have a large impact on total drawing time.
//
//*****************************************************************************


//*****************************************************************************
//
// Include Files
//
//*****************************************************************************
#include "contiki.h"
#include "grlib/grlib.h"
#include "memlcd.h"
#include <string.h>
#include <stdio.h>
   
#include "power.h"
 
#define MLCD_WR 0x01					// MLCD write line command
#define MLCD_CM 0x04					// MLCD clear memory command
#define MLCD_SM 0x00					// MLCD static mode command
#define MLCD_VCOM 0x02					// MLCD VCOM bit

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static process_event_t refresh_event, clear_event;

static const uint8_t clear_cmd[2] = {MLCD_CM, 0};
//static const uint8_t static_cmd[2] = {MLCD_SM, 0};

static struct RefreshData
{
  uint8_t start, end;
}data;


static enum {STATE_NONE, STATE_SENDING}state = STATE_NONE;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
PROCESS(lcd_process, "LCD");

static void SPIInit()
{
  UCB0CTL1 = UCSWRST;

  UCB0CTL0 = UCMODE0 + UCMST + UCSYNC + UCCKPH; // master, 3-pin SPI mode, LSB
  UCB0CTL1 |= UCSSEL__SMCLK; // SMCLK for now
  UCB0BR0 = 4; // 8MHZ / 8 = 1Mhz
  UCB0BR1 = 0;

  //Configure ports.
  LCDSPIDIR |= LCD_SCLK | LCD_SDATA | LCD_SCS;
  LCDSPISEL |= LCD_SCLK | LCD_SDATA;
  LCDSPIOUT &= ~(LCD_SCLK | LCD_SDATA | LCD_SCS);
}

static void SPISend(const void* data, unsigned int size)
{
  PRINTF("Send Data %d bytes\n", size);
  while(UCB0STAT & UCBUSY);
  UCB0CTL1 &= ~UCSWRST;
  state = STATE_SENDING;
  LCDSPIOUT |= LCD_SCS;

  // USB0 TXIFG trigger
  DMACTL0 = DMA0TSEL_19;
  // Source block address
  DMA0SA = (void*)data;
  // Destination single address
  DMA0DA = (void*)&UCB0TXBUF;
  DMA0SZ = size;                                // Block size
  DMA0CTL &= ~DMAIFG;
  DMA0CTL = DMASRCINCR_3 + DMASBDB + DMALEVEL + DMAIE + DMAEN;  // Repeat, inc src
  power_pin(MODULE_LCD);
}

// Initializes the display driver.
// This function initializes the LCD controller
//
// TemplateDisplayFix
void
memlcd_DriverInit(void)
{
  printf("LCD: Initialize...");

  memlcd_InitScreen();

  SPIInit();

  // configure TA0.1 for COM switch
  //TA0CTL |= TASSEL_1 + ID_3 + MC_1;
  //TA0CCTL1 = OUTMOD_7;
  //TA0CCR0 = 4096;
  //TA0CCR1 = 1;

  LCDEXTCOMMSEL &= ~LCDEXTCOMMPIN;
  LCDEXTCOMMDIR |= LCDEXTCOMMPIN; // p4.3 is EXTCOMM

  // enable disply
  LCDENDIR |= LCDENPIN; // p8.2 is display
  LCDENOUT |= LCDENPIN; // set 1 to active display

  // enable 5v
  V5VDIR |= V5BIT;
  V5VOUT |= V5BIT;

  data.start = 0xff;
  data.end = 0;

  process_start(&lcd_process, NULL);
  printf("Done\n");
}

void
memlcd_DriverShutdown(void)
{
  LCDENOUT &= ~LCDENPIN; // set 1 to active display
  V5VOUT &= ~V5BIT;
  UCB0CTL1 = UCSWRST;
}

void halLcdRefresh(int start, int end)
{
  int x = splhigh();
  if (data.start > start)
    data.start = start;
  splx(x);
  if (data.end < end)
    data.end = end;
}

//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.
//!
//! \return None.
//
//*****************************************************************************
void
Template_DriverFlush(void *pvDisplayData)
{
  if (state != STATE_NONE)
    return;
  
  int x = splhigh();
  if (data.start != 0xff)
  {
    splx(x);
    process_post_synch(&lcd_process, refresh_event, &data);
  }
  else
  {
    splx(x);
  }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

int dma_channel_0()
{
  LCDSPIOUT &= ~LCD_SCS;
  power_unpin(MODULE_LCD);
  state = STATE_NONE;
  if (data.start != 0xff)
  {
    process_poll(&lcd_process);
    return 1;
  }
  else
  {
    return 0;
  }
}

PROCESS_THREAD(lcd_process, ev, d)
{
  PROCESS_BEGIN();

  refresh_event = process_alloc_event();
  clear_event = process_alloc_event();

  while(1)
  {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_POLL)
    {
       // if there is an update?
      if (data.start != 0xff)
      {
        SPISend(&lines[data.start], (data.end - data.start + 1)
                * sizeof(linebuf) + 2);
        int x = splhigh();
        data.start = 0xff;
        splx(x);
        data.end = 0;
      }
      else
      {
        while(UCB0STAT & UCBUSY);
        UCB0CTL1 |= UCSWRST;
      }
    }
    else if (ev == refresh_event)
    {
      if (state == STATE_NONE)
      {
        SPISend(&lines[data.start], (data.end - data.start + 1)
                * sizeof(linebuf) + 2);

        int x = splhigh();
        data.start = 0xff;
        splx(x);
        data.end = 0;
      }
    }
    else if (ev == clear_event)
    {
      if (state == STATE_NONE)
      {
        SPISend(clear_cmd, 2);

        int x = splhigh();
        data.start = 0xff;
        splx(x);
        data.end = 0;
      }
    }
  }

  PROCESS_END();
}

void flushlcdsync()
{
  SPISend(&lines[0], (148 + 1) * sizeof(linebuf) + 2);
  power_unpin(MODULE_LCD);
}