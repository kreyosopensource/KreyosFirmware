#include "contiki.h"
#include "uart1.h"
#include "isr_compat.h"

#define DCO_SPEED F_CPU

// 4800 (unused)
#define BAUD_4800 0x01
#define BitTime_4800   (DCO_SPEED / 4800)
#define BitTime_5_4800 (BitTime_4800 / 2)

// 9600
#define BAUD_9600 0x02
#define BitTime_9600   (DCO_SPEED / 9600)
#define BitTime_5_9600 (BitTime_9600 / 2)

// 19200
#define BitTime_19200   (DCO_SPEED / 19200)
#define BitTime_5_19200 (BitTime_19200 / 2)

// 38400
#define BitTime_38400   (DCO_SPEED / 38400)
#define BitTime_5_38400 (BitTime_38400 / 2)
// 57600
#define BitTime_57600   (DCO_SPEED / 57600)
#define BitTime_5_57600 (BitTime_57600 / 2)
// 115200
#define BitTime_115200   (DCO_SPEED / 115200)
#define BitTime_5_115200 (BitTime_115200 / 2)

extern uint8_t uartattached;

void
uart_init(char rate)
{
  UARTOUT &= ~(UARTRXBIT + UARTTXBIT); 
  UARTSEL |= UARTTXBIT + UARTRXBIT;                             // Timer function for TXD/RXD pins
  UARTDIR |= UARTTXBIT;                                         // Set all pins but RXD to output
  UARTDIR &= ~UARTRXBIT;

  TA0CCTL0 = OUTMOD0;                                              // Set TXD Idle as Mark = '1'
//  TA0CCTL1 = SCS + OUTMOD0 + CM1 + CAP + CCIE;                           // Sync, Neg Edge, Capture, Int
  TA0CTL = TASSEL_2 + MC_2;                            // SMCLK, start in continuous mode
}

int putchar(int data)
{
    int tempData;

    if (!uartattached)
        return data;

    int parity_mask = 0x200;
    char bitCount = 0xB;                    // Load Bit counter, 8data + ST/SP +parity
    //int flag;
    //while (TA0CCTL0 & CCIE);                                    // Ensure last char got TX'd

    TA0CCR0 = TA0R;                       // Current state of TA counter
    TA0CCR0 += BitTime_115200;
    tempData = 0x200 + (int)data;           // Add mark stop bit to Data
    tempData = tempData << 1;
    //TZNCCTL_TX = OUTMOD0;

    //int x = splhigh();
    while (bitCount != 0)
    {
        while (!(TA0CCTL0 & CCIFG)) ;
        TA0CCTL0 &= ~CCIFG;
        TA0CCR0 += BitTime_115200;
        if (tempData & 0x01)
        {
            tempData ^= parity_mask;
            TA0CCTL0 &= ~OUTMOD2;         // TX '1'
        }
        else
        {
            TA0CCTL0 |=  OUTMOD2;             // TX '0'
        }

        parity_mask = parity_mask >> 1;
        tempData = tempData >> 1;
        bitCount--;
    }
    while (!(TA0CCTL0 & CCIFG)) ;         // wait for timer
    //splx(x);

    return data;
}