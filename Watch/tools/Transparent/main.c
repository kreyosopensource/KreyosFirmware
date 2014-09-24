
#include <stdint.h>

#include <intrinsics.h>
#include <in430.h>
#include <msp430.h>
#define dint() __disable_interrupt()
#define eint() __enable_interrupt()


#define F_CPU 16000000

#define BT_ACLK_SEL		P11SEL
#define BT_ACLK_DIR		P11DIR
#define BT_ACLK_BIT		BIT0 		// P11.0 ACLK 32Khz -> BT

#define BT_SHUTDOWN_SEL	P10SEL
#define BT_SHUTDOWN_DIR	P10DIR
#define BT_SHUTDOWN_OUT	P10OUT
#define BT_SHUTDOWN_BIT BIT7		// P10.7 nShutdown

// TXD P3.4
#define BT_TXD_OUT      P3OUT
#define BT_TXD_SEL     	P3SEL
#define BT_TXD_DIR     	P3DIR
#define BT_TXD_BIT      BIT4

// RXD P3.5
#define BT_RXD_OUT      P3OUT
#define BT_RXD_SEL     	P3SEL
#define BT_RXD_DIR     	P3DIR
#define BT_RXD_BIT      BIT5

// RTS P1.4
#define BT_RTS_OUT      P1OUT
#define BT_RTS_SEL     	P1SEL
#define BT_RTS_DIR     	P1DIR
#define BT_RTS_BIT      BIT4

// CTS P1.3
#define BT_CTS_OUT      P1OUT
#define BT_CTS_SEL     	P1SEL
#define BT_CTS_DIR     	P1DIR
#define BT_CTS_IFG     	P1IFG
#define BT_CTS_IES     	P1IES
#define BT_CTS_IE     	P1IE
#define BT_CTS_IN       P1IN
#define BT_CTS_BIT      BIT3

//P9.7 nOE SLOW clk
#define OECLKDIR		P9DIR
#define OECLKOUT		P9OUT
#define OECLKBIT		BIT7

//P9.2 nOE HCI 
#define OEHCIDIR		P9DIR
#define OEHCIOUT		P9OUT
#define OEHCIBIT		BIT2

void config_bt()
{
    // enable power
  OECLKDIR |= OECLKBIT;
  OECLKOUT &= ~OECLKBIT;

  OEHCIDIR |= OEHCIBIT;
  OEHCIOUT &= ~OEHCIBIT;

  // set BT RTS
  BT_RTS_SEL &= ~BT_RTS_BIT;  // = 0 - I/O
  BT_RTS_DIR |=  BT_RTS_BIT;  // = 1 - Output
  BT_RTS_OUT &=  ~BT_RTS_BIT;  // = 1 - RTS high -> stop

  // set BT CTS
  BT_CTS_SEL &= ~BT_CTS_BIT;  // = 0 - I/O
  BT_CTS_DIR &= ~BT_CTS_BIT;  // = 0 - Input

  UCA0CTL1 |= UCSWRST;              //Reset State

  BT_TXD_SEL |= BT_TXD_BIT;
  BT_TXD_DIR |= BT_TXD_BIT;

  BT_RXD_SEL |= BT_RXD_BIT;
  BT_RXD_DIR &= ~BT_RXD_BIT;

  UCA0CTL0 = 0;
  UCA0CTL1 |= UCSSEL_2;

  UCA0BR0 = 138;  // from family user guide
  UCA0BR1 = 0;
  UCA0MCTL= UCBRS_7;  // + 0.875
    
  UCA0CTL1 &= ~UCSWRST;             // continue
  UCA0IE   |= UCRXIE;
  
  BT_CTS_IFG  &=  ~BT_CTS_BIT;     // no IRQ pending
  BT_CTS_IES &= ~BT_CTS_BIT;  // IRQ on 0->1 transition
  BT_CTS_IE  |=  BT_CTS_BIT;  // enable IRQ for P1.3

}

#define UARTOUT P1OUT
#define UARTSEL P1SEL
#define UARTDIR P1DIR
#define UARTIES P1IES
#define UARTIE  P1IE
#define UARTIFG P1IFG
#define UARTIN  P1IN

#define UARTTXBIT BIT1
#define UARTRXBIT BIT2

#define DCO_SPEED F_CPU

// 115200
#define BitTime_115200   (DCO_SPEED / 115200)
#define BitTime_5_115200 (BitTime_115200 / 2)

static uint16_t BitTime;
static uint16_t BitTime_5;

static int uartTxDaTA0;                                         // UART internal variable for TX

extern void msp430_cpu_init(void);

void config_usb()
{
  UARTOUT &= ~(UARTRXBIT + UARTTXBIT); 
  UARTSEL |= UARTTXBIT + UARTRXBIT;                             // Timer function for TXD/RXD pins
  UARTDIR |= UARTTXBIT;                                         // Set all pins but RXD to output
  UARTDIR &= ~UARTRXBIT;

  TA0CCTL0 = OUT;                                              // Set TXD Idle as Mark = '1'
  TA0CCTL1 = SCS + OUTMOD0 + CM1 + CAP + CCIE;                           // Sync, Neg Edge, Capture, Int
  TA0CTL = TASSEL_2 + MC_2 + TACLR;                            // SMCLK, start in continuous mode

  BitTime = BitTime_115200;
  BitTime_5 = BitTime_5_115200;
}

#define LIGHTDIR P4DIR
#define LIGHTOUT P4OUT
#define LIGHTSEL P4SEL
#define LIGHT    BIT2

void config_light()
{
  LIGHTDIR |= LIGHT;
  LIGHTSEL &= ~LIGHT;

  LIGHTOUT |= LIGHT;
}

#define BUTTON BIT2

void config_key()
{
    P2DIR &= ~BUTTON;
    P2SEL &= ~BUTTON;
    P2IES |= BUTTON;
    P2REN |= BUTTON;
    P2OUT |= BUTTON;
    P2IFG &= ~BUTTON;
    P2IE |= BUTTON;
}

void uart_sendByte(uint8_t byte)
{
  while (TA0CCTL0 & CCIE);                                    // Ensure last char got TX'd
  TA0CCR0 = TA0R;                                              // Current state of TA counter
  TA0CCR0 += BitTime;                                      // One bit time till first bit
  TA0CCTL0 = OUTMOD0 + CCIE;                                  // Set TXD on EQU0, Int
  uartTxDaTA0 = byte;                                         // Load global variable
  uartTxDaTA0 |= 0x100;                                       // Add mark stop bit to TXData
  uartTxDaTA0 <<= 1;                                          // Add space start bit
}

static uint8_t USBSTAT;
static uint8_t output;

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  msp430_cpu_init();
  
  __delay_cycles(32000);                   
  __delay_cycles(32000);                   
  
  
  // configure BT
  config_bt();
  
  // config usb
  config_usb();

  // config key
  config_key();
  
  // config the back light
  config_light();

  // set BT SHUTDOWN to 1 (active low)
  BT_SHUTDOWN_SEL &= ~BT_SHUTDOWN_BIT;  // = 0 - I/O
  BT_SHUTDOWN_DIR |=  BT_SHUTDOWN_BIT;  // = 1 - Output
  BT_SHUTDOWN_OUT &=  ~BT_SHUTDOWN_BIT;  // = 0
  
  // Enable ACLK to provide 32 kHz clock to Bluetooth module
  BT_ACLK_SEL |= BT_ACLK_BIT;
  BT_ACLK_DIR |= BT_ACLK_BIT;

  // wait clock stable
  //__delay_cycles(3200000);                // Delay  
  __delay_cycles(2000);                // Delay  
  BT_SHUTDOWN_OUT |=  BT_SHUTDOWN_BIT;  // = 1
  
  // sending
  for(;;)
  {
    if (USBSTAT)
    {
      uint8_t data;
      dint();
      data = output;
      USBSTAT = 0;
      eint();
      uart_sendByte(data);
      //BT_RTS_OUT &=  ~BT_RTS_BIT;  // = 1 - RTS high -> stop
    }
    else
    {
      __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    }
  }
}

/**
* ISR for TXD and RXD
*/
#define RXBITCNTWITHSTOP 9
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0_a1_interrupt(void)
{
  static int8_t rxBitCnt = RXBITCNTWITHSTOP;
  static int rxData = 0;

  switch (TA0IV)                      
  {
  case 2:                                      // TACCR1 CCIFG - UART RX
      TA0CCR1 += BitTime;                              // Add Offset to CCRx
      if (TA0CCTL1 & CAP)                                 // Capture mode = start bit edge
      {
          TA0CCTL1 &= ~CAP;                               // Switch capture to compare mode
          TA0CCR1 += BitTime_5;                    // Point CCRx to middle of D0
      }
      else 
      {
          rxData >>= 1;
          if (TA0CCTL1 & SCCI)                            // Get bit waiting in receive latch
          {  
              rxData |= 0x100;
          }
          rxBitCnt--;
          if (rxBitCnt == 0)                              // All bits RXed?
          {
              TA0CCTL1 |= CAP;                            // Switch compare to capture mode
              char RXByte = rxData & 0xFF;                      // Store in global variable
              rxBitCnt = RXBITCNTWITHSTOP;                // Re-load bit counter
              rxData = 0;
              
              while (!(UCA0IFG&UCTXIFG));
              UCA0TXBUF = RXByte; 
          }
      }
      break;

  default:
      break;
  }
 
}


//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
    static unsigned char txBitCnt = 10;

    TA0CCR0 += BitTime;                                      // Add Offset to CCRx
    if (txBitCnt == 0)                                          // All bits TXed?
    {    
        TA0CCTL0 &= ~CCIE;                                      // All bits TXed, disable interrupt
        txBitCnt = 10;                                          // Re-load bit counter
    }
    else 
    {
        if (uartTxDaTA0 & 0x01) 
        {
          TA0CCTL0 &= ~OUTMOD2;                                 // TX Mark '1'
        }
        else 
        {
          TA0CCTL0 |= OUTMOD2;                                  // TX Space '0'
        }
        uartTxDaTA0 >>= 1;
        txBitCnt--;
    }
}

// Confirm TX buffer is ready and forward RX character.
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI0_ISR(void)
{
 // BT_RTS_OUT |=  BT_RTS_BIT;  // = 1 - RTS high -> stop
  output = UCA0RXBUF;                     // USCI_A1 RX -> USCI_A2 TX
  USBSTAT = 1;
  LPM4_EXIT;
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch(__even_in_range(P1IV, 16))
    {
       case 0: break;
  case 2:
    {
      break;                          // Pin0
    }
  case 4:
    {
      break;                          // Pin1
    }
  case 6:
    {
      break;                          // Pin2
    }
  case 8: 
    {
     LPM4_EXIT;
      break;                          // Pin3
    }
  case 10: break;                          // Pin4
  case 12: break;                          // Pin5
  case 14: break;
  case 16: break;                          // Pin7
  default: break;
  }
}

#define NMI_VECTOR 2
#pragma vector=NMI_VECTOR
__interrupt void _NMIISR(void)
{

}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    switch(__even_in_range(P2IV, 16))
    {
       case 0: break;
  case 2:
    { 
      break;                          // Pin0
    }
  case 4:
    {
      break;                          // Pin1
    }
  case 6:
    {
      if (LIGHTOUT & LIGHT)
      {
        // light is on, we are enabled
        LIGHTOUT &= ~LIGHT;
        
        SFRRPCR &= ~(SYSRSTRE + SYSRSTUP + SYSNMI);
        SFRIE1 |= NMIIE;
      }
      else
      {
        // light is off, we are disabled
        LIGHTOUT |= LIGHT;
        
        SFRRPCR |= (SYSRSTRE + SYSRSTUP + SYSNMI);
        SFRIE1 &= ~NMIIE;
      }
      
      break;                          // Pin2
    }
  case 8: 
    {
     LPM4_EXIT;
      break;                          // Pin3
    }
  case 10: break;                          // Pin4
  case 12: break;                          // Pin5
  case 14: break;
  case 16: break;                          // Pin7
  default: break;
  }
}