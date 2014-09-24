#include "contiki.h"
#include "battery.h"
#include "isr_compat.h"
#include <stdio.h>

static uint16_t level;
uint8_t uartattached = 1;

static void setoutputfloat()
{
  // set high impredence for BAT_OUT
  BATOUTDIR &= ~BATOUTPIN;
  BATOUTREN &= ~BATOUTPIN;
}

static void setoutputhigh()
{
  BATOUTDIR |= BATOUTPIN;
  BATOUTOUT |= BATOUTPIN;
}

/* static void setoutputlow()
{
  BATOUTDIR |= BATOUTPIN;
  BATOUTOUT |= BATOUTPIN;
}*/

void battery_init(void)
{
  // set input for BAT_IN
  BATINDIR &= ~BATINPIN;

  BATINSEL |= BATLEVEL;

  REFCTL0 |= REFMSTR + REFVSEL_2 + REFON;
  ADC12CTL0 = ADC12ON + ADC12SHT0_12;
  //ADC12CTLADC12SHP1 = ADC12SSEL_0 + ADC12SHP;
  ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHP + ADC12SSEL_1; 
  //ADC12CTL2 = ADC12TCOFF + ADC12RES_2 + ADC12REFBURST;
  ADC12IE = ADC12IE0;                           // Enable interrupt
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_4 + ADC12EOS;      // A4 as input

  ADC12CTL0 |= ADC12ENC;
  __delay_cycles(75 * 8);
  ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

  setoutputfloat();
}

BATTERY_STATE battery_state(void)
{
  setoutputfloat();
  __delay_cycles(10);
  if (BATININ & BATINPIN) // if it is high
  {
    uartattached = 0;
    return BATTERY_STATE_DISCHARGING;
  }

  uartattached = 1;
  setoutputhigh();
  __delay_cycles(10);
  int instate = (BATININ & BATINPIN) != 0; // if it is high
 
  if (instate)
    return BATTERY_STATE_FULL;
  else
    return BATTERY_STATE_CHARGING;
}

// map battery level to 0-15 scale
static const uint16_t charge_curve[] = 
{
  3038, 3080, 3113, 3131, 3159, 3199, 3251, 3306, 3373, 3447
};

static const uint16_t discharge_curve[] = 
{
  2910, 3011, 3048, 3078, 3109, 3137, 3175, 3233, 3292, 3398
};



/* return battery level from 0 - 9 */
uint8_t battery_level(BATTERY_STATE state)
{
//  printf("battery level : %d\n", level);
  if (!(ADC12CTL1 & ADC12BUSY))
    ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion

  printf("level:%d\n", level);
  uint8_t ret;

  switch(state)
  {
    case BATTERY_STATE_FULL:
      return 9;
    case BATTERY_STATE_CHARGING:
    {
      for(ret = 1; ret < sizeof(charge_curve); ret++)
      {
        if (level < charge_curve[ret])
          return ret - 1;
      }

      return 9;
    }
    case BATTERY_STATE_DISCHARGING:
    {
      for(ret = 1; ret < sizeof(discharge_curve); ret++)
      {
        if (level < discharge_curve[ret])
          return ret - 1;
      }

      return 9;    
    }
  }

  return 0;
}

ISR(ADC12, _ADC12_ISR)
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    level = ADC12MEM0;
    break;
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}