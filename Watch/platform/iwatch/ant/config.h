/////////////////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING EXAMPLE CODE IS INTENDED FOR LIMITED CIRCULATION ONLY.
//
// Please forward all questions regarding this code to ANT Technical Support.
//
// Dynastream Innovations Inc.
// 228 River Avenue
// Cochrane, Alberta, Canada
// T4C 2C1
//
// (P) (403) 932-9292
// (F) (403) 932-6521
// (TF) 1-866-932-9292
// (E) support@thisisant.com
//
// www.thisisant.com
//
// Reference Design Disclaimer
//
// The references designs and codes provided may be used with ANT devices only and remain the copyrighted property of
// Dynastream Innovations Inc. The reference designs and codes are being provided on an "as-is" basis and as an accommodation,
// and therefore all warranties, representations, or guarantees of any kind (whether express, implied or statutory) including,
// without limitation, warranties of merchantability, non-infringement,
// or fitness for a particular purpose, are specifically disclaimed.
//
// ©2008 Dynastream Innovations Inc. All Rights Reserved
// This software may not be reproduced by
// any means without express written approval of Dynastream
// Innovations Inc.
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef CONFIG_H
#define CONFIG_H

//------------------------------------------------------------------------------
// DEVELOPMENT BOARD TYPE
//#define BOARD_EZ430_RF2500 1
#define BOARD_ANT_CPU 1
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SERIAL PORT TYPE
#define USE_UART_DEBUG
#define SERIAL_UART_ASYNC

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// EXTERNAL CLOCK
//#define USE_EXTERNAL_32KHZ
//------------------------------------------------------------------------------


// Compiler Environment
#include "msp430.h"

// UCI Registers fro asynchronous.
#define UART_ASYNC_UCI_RXBUF                 UCA2RXBUF      // UCI Recieve Buffer Register
#define UART_ASYNC_UCI_TXBUF                 UCA2TXBUF      // UCI Transmit Buffer Register
#define UART_ASYNC_UCI_CTL0                  UCA2CTL0       // UCI Control Register 0
#define UART_ASYNC_UCI_CTL1                  UCA2CTL1       // UCI Control Register 1
#define UART_ASYNC_UCI_BR0                   UCA2BR0        // UCI Baud Rate Register 0
#define UART_ASYNC_UCI_BR1                   UCA2BR1        // UCI Baud Rate Register 1
#define UART_ASYNC_UCI_MCTL                  UCA2MCTL       // UCI Modulation Control Register

#define UART_ASYNC_UCI_IER                   UCA2IE         // Interrupt enable register
#define UART_ASYNC_UCI_IFR                   UCA2IFG        // Interrupt flag register

#define UART_ASYNC_RX_INT                    UCRXIE         // Recieve interrupt bit
#define UART_ASYNC_TX_INT                    UCTXIE         // Transmit interrupt bit


// Serial Port Definitions (Common)
#if 0
//------------------------------------------------------------------------------
// PORTSEL
#define SERIAL_PORTSEL_SEL                   P4SEL
#define SERIAL_PORTSEL_DIR                   P4DIR
#define SERIAL_PORTSEL_BIT                   BIT6           // P4.6
#define SERIAL_PORTSEL_OUT                   P4OUT

//------------------------------------------------------------------------------
// SFLOW
#define SERIAL_SFLOW_SEL                     P4SEL
#define SERIAL_SFLOW_DIR                     P4DIR
#define SERIAL_SFLOW_BIT                     BIT5           // P4.5
#define SERIAL_SFLOW_OUT                     P4OUT

//------------------------------------------------------------------------------
// IOSEL
#define SERIAL_IOSEL_SEL                     P2SEL          // Interrupt Flag
#define SERIAL_IOSEL_DIR                     P2DIR
#define SERIAL_IOSEL_OUT                     P2OUT
#define SERIAL_IOSEL_BIT                     BIT1            // P2.1
#endif

// Asynchronous
#define ASYNC_RX_DIR                         P9DIR
#define ASYNC_RX_SEL                         P9SEL
#define ASYNC_RX_BIT                         BIT5           // P9.5
#define ASYNC_RX_IN                          P9IN           // TX on ANT

#define ASYNC_TX_DIR                         P9DIR
#define ASYNC_TX_SEL                         P9SEL
#define ASYNC_TX_BIT                         BIT4           // P9.4
#define ASYNC_TX_OUT                         P9OUT          // RX on ANT

#define ASYNC_RTS_DIR                        P1DIR
#define ASYNC_RTS_SEL                        P1SEL
#define ASYNC_RTS_BIT                        BIT5           // P1.5
#define ASYNC_RTS_IN                         P1IN           // RTS on ANT

#define ASYNC_SUSPEND_DIR                    P4DIR
#define ASYNC_SUSPEND_SEL                    P4SEL
#define ASYNC_SUSPEND_BIT                    BIT5           // P4.5
#define ASYNC_SUSPEND_OUT                    P4OUT          // SUSPEND on ANT

#define ASYNC_SLEEP_DIR                      P4DIR
#define ASYNC_SLEEP_SEL                      P4SEL
#define ASYNC_SLEEP_BIT                      BIT4           // P4.4
#define ASYNC_SLEEP_OUT                      P4OUT          // SLEEP on ANT

//------------------------------------------------------------------------------
// ANT Reset
#define SYSTEM_RST_SEL                       P4SEL
#define SYSTEM_RST_DIR                       P4DIR
#define SYSTEM_RST_BIT                       BIT6           // P4.6
#define SYSTEM_RST_OUT                       P4OUT

#define SYNC_SEN_IFG		                     P1IFG
#define SYNC_SEN_BIT			                   BIT5			//P1.5
#define SYNC_SEN_IES                         P1IES
#define SYNC_SEN_IE                          P1IE

#endif
