
#include "contiki.h"
#include "isr_compat.h"

extern int dma_channel_0();
extern int dma_channel_1();
#define NMI_VECTOR 2

ISR(NMI, _NMIISR)
{

}

ISR(DMA, _DMA0ISR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  switch(__even_in_range(DMAIV,16))
  {
  case 0: break;
  case 2:                                 // DMA0IFG = DMA Channel 0
    {
      if (dma_channel_0())
        LPM4_EXIT;
      break;
    }
  case 4:                                 // DMA1IFG = DMA Channel 1
    {
      if (dma_channel_1())
        LPM4_EXIT;
      break;
    }
  case 6: break;                          // DMA2IFG = DMA Channel 2
  case 8: break;                          // DMA3IFG = DMA Channel 3
  case 10: break;                         // DMA4IFG = DMA Channel 4
  case 12: break;                         // DMA5IFG = DMA Channel 5
  case 14: break;                         // DMA6IFG = DMA Channel 6
  case 16: break;                         // DMA7IFG = DMA Channel 7
  default: break;
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

extern int port1_pin3();
extern int port1_pin5();
extern int port1_pin6();

ISR(PORT1, _PORT1ISR) {
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  switch(__even_in_range(P1IV, 16))
  {
  case 0: break;
  case 2: break;                          // Pin0
  case 4: break;                          // Pin1
  case 6: break;                          // Pin2
  case 8:                                 // Pin3
    {
      if (port1_pin3())
        LPM4_EXIT;
      break;
    }
  case 10: break;                          // Pin4
#if PRODUCT_W001
  case 12:                                 // Pin5
      {
      if (port1_pin5())
        LPM4_EXIT;
      break;
    }
  case 14:                                 // Pin6
    {
      if (port1_pin6())
        LPM4_EXIT;
      break;
    }
#endif
  case 16: break;                          // Pin7
  default: break;
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

extern int port2_pin0();
extern int port2_pin1();
extern int port2_pin2();
extern int port2_pin6();

ISR(PORT2, _PORT2ISR)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  switch(__even_in_range(P2IV, 16))
  {
  case 0: break;
  case 2:
    {
      if (port2_pin0())
        LPM4_EXIT;

      break;                          // Pin0
    }
  case 4:
    {
      if (port2_pin1())
        LPM4_EXIT;

      break;                          // Pin1
    }
  case 6:
    {
      if (port2_pin2())
        LPM4_EXIT;

      break;                          // Pin2
    }
  case 8: break;                          // Pin3
  case 10: break;                          // Pin4
  case 12: break;                          // Pin5
  case 14:
    {
      if (port2_pin6())
        LPM4_EXIT;

      break;                          // Pin6
    }
  case 16: break;                          // Pin7
  default: break;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}