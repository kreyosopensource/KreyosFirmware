
#include <stdint.h>
#include "contiki.h"
#include "btstack-config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "isr_compat.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "bluetooth.h"

#include "hal_compat.h"
#include <btstack/hal_uart_dma.h>

#include "power.h"

// rx state
static uint16_t  bytes_to_read = 0;
static uint8_t * rx_buffer_ptr = 0;
static uint8_t   triggered;

static uint8_t   rx_temp_buffer;
static uint8_t   rx_temp_size = 0;

// handlers
static void (*rx_done_handler)(void) = NULL;
static void (*tx_done_handler)(void) = NULL;
static void (*cts_irq_handler)(void) = NULL;

/**
* @brief  Initializes the serial communications peripheral and GPIO ports
*         to communicate with the PAN BT .. assuming 16 Mhz CPU
*
* @param  none
*
* @return none
*/
void hal_uart_dma_init(void)
{
  // set BT RTS
  BT_RTS_SEL &= ~BT_RTS_BIT;  // = 0 - I/O
  BT_RTS_DIR |=  BT_RTS_BIT;  // = 1 - Output
  BT_RTS_OUT |=  BT_RTS_BIT;  // = 1 - RTS high -> stop

  // set BT CTS
  BT_CTS_SEL &= ~BT_CTS_BIT;  // = 0 - I/O
  BT_CTS_DIR &= ~BT_CTS_BIT;  // = 0 - Input
  BT_CTS_IES &= ~BT_CTS_BIT;  // IRQ on 0->1 transition

  UCA0CTL1 |= UCSWRST;              //Reset State

  BT_TXD_SEL |= BT_TXD_BIT;
  BT_TXD_DIR |= BT_TXD_BIT;

  BT_RXD_SEL |= BT_RXD_BIT;
  BT_RXD_DIR &= ~BT_RXD_BIT;

  UCA0CTL0 = UCMODE_0;
  UCA0CTL1 |= UCSSEL_2;

  UCA0CTL1 &= ~UCSWRST;             // continue
  hal_uart_dma_set_baud(115200);
  UCA0IE |= UCRXIE ;  // enable RX interrupts

  power_pin(MODULE_BT);
}

/**

UART used in low-frequency mode
In this mode, the maximum USCI baud rate is one-third the UART source clock frequency BRCLK.

16000000 /  576000 = 277.77
16000000 /  115200 = 138.88
16000000 /  921600 =  17.36
16000000 / 1000000 =  16.00
16000000 / 2000000 =   8.00
16000000 / 2400000 =   6.66
16000000 / 3000000 =   3.33
16000000 / 4000000 =   2.00

*/
int hal_uart_dma_set_baud(uint32_t baud){
  int result = 0;

  while(UCA0STAT & UCBUSY);
  UCA0CTL1 |= UCSWRST;              //Reset State

  switch (baud){

  case 4000000:
    UCA0BR0 = 2;
    UCA0BR1 = 0;
    UCA0MCTL= 0 << 1;  // + 0.000
    break;

  case 2000000:
    UCA0BR0 = 4;
    UCA0BR1 = 0;
    UCA0MCTL= 0 << 1;  // + 0.000
    break;

  case 1000000:
    UCA0BR0 = 8;
    UCA0BR1 = 0;
    UCA0MCTL= 0 << 1;  // + 0.000
    break;

  case 500000:
    UCA0BR0 = 16;
    UCA0BR1 = 0;
    UCA0MCTL= 0 << 1;  // + 0.000
    break;

  case 115200:
    UCA0BR0 = 69;  // from family user guide
    UCA0BR1 = 0;
    UCA0MCTL= 4 << 1;
    break;

  default:
    result = -1;
    break;
  }

  UCA0CTL1 &= ~UCSWRST;             // continue

  return result;
}

void hal_uart_dma_set_block_received( void (*the_block_handler)(void)){
  rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void)){
  tx_done_handler = the_block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void)){
  if (the_irq_handler){
    BT_CTS_IFG  &=  ~BT_CTS_BIT;     // no IRQ pending
    BT_CTS_IE  |=  BT_CTS_BIT;  // enable IRQ for P1.3
  }
  else
  {
    BT_CTS_IE &= ~BT_CTS_BIT;
  }

  cts_irq_handler = the_irq_handler;
}

/**********************************************************************/
/**
* @brief  Disables the serial communications peripheral and clears the GPIO
*         settings used to communicate with the BT.
*
* @param  none
*
* @return none
**************************************************************************/
void hal_uart_dma_shutdown(void) {
  printf("hal_uart_dma_shutdown");
  

  UCA0IE &= ~(UCRXIE | UCTXIE);
  UCA0CTL1 = UCSWRST;                          //Reset State
  
  BT_RXD_SEL &= ~BT_RXD_BIT;
  BT_TXD_SEL &= ~BT_TXD_BIT;

  BT_TXD_DIR |= BT_TXD_BIT;
  BT_RXD_DIR |= BT_RXD_BIT;

  BT_TXD_OUT &= ~BT_TXD_BIT;
  BT_RXD_OUT &= ~BT_RXD_BIT;
}

int dma_channel_1()
{
  if (tx_done_handler)
    (*tx_done_handler)();

  if (triggered)
  {
    triggered = 0;
    return 1;
  }
  else
    return 0;
}

void hal_uart_dma_send_block(const uint8_t * data, uint16_t len){

  //log_info("hal_uart_dma_send_block, size %u\n\r", len);

  // UCA0 TXIFG trigger
  DMACTL0 |= DMA1TSEL_17;
  // Source block address
  DMA1SA = (void*)data;
  // Destination single address
  DMA1DA = (void*)&UCA0TXBUF;
  DMA1SZ = len;                                // Block size
  DMA1CTL &= ~DMAIFG;
  DMA1CTL = DMASRCINCR_3 + DMASBDB + DMALEVEL + DMAIE + DMAEN;  // Repeat, inc src
}

// int used to indicate a request for more new data
void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len){

  //log_info("receive_block size %u temp: %u\n", len, rx_temp_size);

  int x = splhigh();
  if (rx_temp_size)
  {
    *buffer = rx_temp_buffer;
    rx_temp_size = 0;
    buffer++;
    len--;
  }

  if (len == 0)
  {
    if (rx_done_handler)
      (*rx_done_handler)();
  }
  else
  {
    rx_buffer_ptr = buffer;
    bytes_to_read = len;
    UCA0IE |= UCRXIE;    // enable RX interrupts

    // enable send
    BT_RTS_OUT &= ~BT_RTS_BIT;  // = 0 - RTS low -> ok
  }

  splx(x);
}

void hal_uart_dma_set_sleep(uint8_t sleep)
{
  if (sleep)
  {
    // wait for last byte sent out
    while(UCA0STAT & UCBUSY);
    UCA0IE &= ~(UCRXIE | UCTXIE);
    UCA0CTL1 |= UCSWRST;                          //Reset State

    power_unpin(MODULE_BT);
  }
  else
  {
    UCA0IE |= UCRXIE | UCTXIE;
    UCA0CTL1 &= ~UCSWRST;                          //Reset State
    power_pin(MODULE_BT);
  }
  
  triggered = 1;
}

// block-wise "DMA" RX/TX UART driver
ISR(USCI_A0, usbRxTxISR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  // find reason
  switch (__even_in_range(UCA0IV, 16)){

  case 2: // RXIFG
  {
    uint8_t data = UCA0RXBUF;
    if (bytes_to_read == 0) {
      BT_RTS_OUT |= BT_RTS_BIT;      // = 1 - RTS high -> stop
      UCA0IE &= ~UCRXIE ;  // disable RX interrupts

      // put the data into buffer to avoid race condition
      rx_temp_buffer = data;
      rx_temp_size++;
      return;
    }

    *rx_buffer_ptr = data;
    ++rx_buffer_ptr;
    --bytes_to_read;
    if (bytes_to_read > 0) {
      break;
    }
    BT_RTS_OUT |= BT_RTS_BIT;      // = 1 - RTS high -> stop

    if (rx_done_handler)
      (*rx_done_handler)();

    if (triggered)
    {
      triggered = 0;
      UCA0IE &= ~UCRXIE ;  // disable RX interrupts
      LPM4_EXIT;
    }
  }
    break;

  default:
    break;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}


// CTS ISR
int port1_pin3()
{
  if (cts_irq_handler)
    (*cts_irq_handler)();

  if (triggered)
  {
    triggered = 0;
    return 1;
  }
  else  
    return 0;
}

void embedded_trigger(void)
{
  process_poll(&bluetooth_process);
  triggered = 1;
}
