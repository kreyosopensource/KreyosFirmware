#include "contiki.h"
#include "i2c.h"
#include "isr_compat.h"
#include "sys/rtimer.h"
#include <stdio.h>

static uint8_t *rxdata;
static uint16_t rxlen;
static const uint8_t *payload;
static uint8_t payloadlen;
static const uint8_t *txdata;
static uint8_t txlen;

static enum { STATE_IDL, STATE_RUNNING, STATE_DONE, STATE_ERROR} state;

void I2C_Init()
{
  printf("I2C: Initialize...");
  // initialize i2c UCB1
  UCB1CTL1 |= UCSWRST;

  UCB1CTL0 = UCMODE_3 + UCMST + UCSYNC; // master, I2c mode, LSB
  UCB1CTL1 = UCSSEL__SMCLK + UCSWRST; // SMCLK for now
  UCB1BR0 = 20; // 8MHZ / 80 = 100Khz
  UCB1BR1 = 0;

  // give clk signal to all slaves and wait SDA go back to high
  P3DIR &= ~BIT7; // SDA -> INPUT
  P5DIR |= BIT4;
  while((P3IN & BIT7) == 0)
  {
    printf(".");
    P5OUT |= BIT4;
    printf("=");
    P5OUT &= ~BIT4;
  }

  __delay_cycles(300);
  //Configure ports.
  P3SEL |= BIT7;
  P5SEL |= BIT4;

  UCB1CTL1 &= ~UCSWRST;
  printf("\n$$OK I2C\n");
}

void I2C_shutdown()
{
  UCB1CTL1 |= UCSWRST;
}

int I2C_readbytes(unsigned char reg, unsigned char *data, uint16_t len)
{
  txdata = &reg;
  txlen = 1;

  rxdata = data;
  rxlen = len;

  state = STATE_RUNNING;
  UCB1IE |= UCTXIE + UCRXIE + UCNACKIE;              // Enable TX interrupt

  while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

  BUSYWAIT_UNTIL(state != STATE_RUNNING, RTIMER_SECOND/8);

  UCB1IE &= ~(UCTXIE + UCRXIE);

  return state != STATE_DONE;
}

int I2C_writebytes(unsigned char reg, const unsigned char *data, uint8_t len)
{
  txdata = &reg;
  txlen = 1;

  payload = data;
  payloadlen = len;

  rxlen = 0;

  state = STATE_RUNNING;
  UCB1IE |= UCTXIE | UCNACKIE;                         // Enable TX interrupt

  while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

  BUSYWAIT_UNTIL(state != STATE_RUNNING, RTIMER_SECOND/8);

  UCB1IE &= ~UCTXIE;

  return state != STATE_DONE;
}

int I2C_write(unsigned char reg, unsigned char data)
{
  txdata = &reg;
  txlen = 1;

  payload = &data;
  payloadlen = 1;

  rxlen = 0;

  state = STATE_RUNNING;
  UCB1IE |= UCTXIE | UCNACKIE;                         // Enable TX interrupt

  while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
  UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

  BUSYWAIT_UNTIL(state != STATE_RUNNING, RTIMER_SECOND/8);

  UCB1IE &= ~UCTXIE;

  return state != STATE_DONE;
}

void  I2C_addr(unsigned char address)
{
  if (UCB1I2CSA == address)
    return;

  UCB1CTL1 |= UCSWRST;
  UCB1I2CSA = address;
  UCB1CTL1 &= ~UCSWRST;
}

void I2C_done()
{
  while(UCB1STAT & UCBUSY);
}

ISR(USCI_B1, USCI_B1_ISR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: 
    LPM4_EXIT;
    break;                           // Vector  4: NACKIFG
  case  6:    		                    // Vector  6: STTIFG
    {
      break;
    }
  case  8:      // Vector  8: STPIFG
    {
      break;
    }
  case 10:                                  // Vector 10: RXIFG
    {
      rxlen--;
      if (rxlen)
      {
        if (rxlen == 1)
        {
          UCB1CTL1 |= UCTXSTP;
        }
      }
      else
      {
        UCB1IFG &= ~UCRXIFG;
        state = STATE_DONE;
        LPM4_EXIT;
      }

      // read data to release SCL
      *rxdata++ = UCB1RXBUF;
      break;
    }
  case 12:                                  // Vector 12: TXIFG
    if (txlen)                          // Check TX byte counter
    {
      UCB1TXBUF = *txdata++;               // Load TX buffer
      txlen--;                          // Decrement TX byte counter

      if (txlen == 0 && payloadlen)
      {
        txlen = payloadlen;
        txdata = payload;
        payloadlen = 0;
      }
    }
    else
    {
      UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      // done, give stop flag
      if (rxlen == 0) // only do TX
      {
        UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
        state = STATE_DONE;
        LPM4_EXIT;
      }
      else
      {
        UCB1CTL1 &= ~UCTR;         			// I2C RX
        UCB1CTL1 |= UCTXSTT;         		// I2C start condition
        if (rxlen == 1)
        {
          // wait Start signal send out
          while(UCB1CTL1 & UCTXSTT);
          // then send NACK and stop
          UCB1CTL1 |= UCTXSTP;
        }
      }
    }
    break;
  default: break;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
