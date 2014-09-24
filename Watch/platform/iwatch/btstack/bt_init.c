#include "contiki.h"
#include "bluetooth.h"

#include <stdio.h>

void bluetooth_platform_init(void)
{
  // enable power
  BTPOWERDIR |= BTPOWERBIT;
  BTPOWEROUT |= BTPOWERBIT;

  OECLKDIR |= OECLKBIT;
  OECLKOUT &= ~OECLKBIT;

  OEHCIDIR |= OEHCIBIT;
  OEHCIOUT &= ~OEHCIBIT;

  // set BT RTS
  BT_RTS_SEL &= ~BT_RTS_BIT;  // = 0 - I/O
  BT_RTS_DIR |=  BT_RTS_BIT;  // = 1 - Output
  BT_RTS_OUT |=  BT_RTS_BIT;  // = 1 - RTS high -> stop

  // set BT CTS
  BT_CTS_SEL &= ~BT_CTS_BIT;  // = 0 - I/O
  BT_CTS_DIR &= ~BT_CTS_BIT;  // = 0 - Input
  BT_CTS_IES &= ~BT_CTS_BIT;  // IRQ on 0->1 transition

  // set BT SHUTDOWN to 1 (active low)
  BT_SHUTDOWN_SEL &= ~BT_SHUTDOWN_BIT;  // = 0 - I/O
  BT_SHUTDOWN_DIR |=  BT_SHUTDOWN_BIT;  // = 1 - Output
  BT_SHUTDOWN_OUT &=  ~BT_SHUTDOWN_BIT;  // = 0

  // Enable ACLK to provide 32 kHz clock to Bluetooth module
  BT_ACLK_SEL |= BT_ACLK_BIT;
  BT_ACLK_DIR |= BT_ACLK_BIT;

  // wait clock stable
  __delay_cycles(1000);

  BT_SHUTDOWN_OUT |=  BT_SHUTDOWN_BIT;  // = 1
}


void bluetooth_platform_shutdown(void)
{
  printf("bluetooth process exit\n");
  BT_SHUTDOWN_OUT &=  ~BT_SHUTDOWN_BIT;  // = 0

  // disable power
  BTPOWEROUT &= ~BTPOWERBIT;

  BT_ACLK_SEL &= ~BT_ACLK_BIT;
  
  OECLKOUT |= OECLKBIT;
  OEHCIOUT |= OEHCIBIT;
}