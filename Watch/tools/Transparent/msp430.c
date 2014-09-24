#include <intrinsics.h>
#include <in430.h>
#include <msp430.h>
#define dint() __disable_interrupt()
#define eint() __enable_interrupt()


   /* The following function is a utility function the is used to       */
   /* increment the VCore setting to the specified value.               */
static unsigned char IncrementVCORE(unsigned char Level)
{
   unsigned char Result;
   unsigned char PMMRIE_backup;
   unsigned char SVSMHCTL_backup;
   unsigned char SVSMLCTL_backup;

   /* The code flow for increasing the Vcore has been altered to work   */
   /* around the erratum FLASH37.  Please refer to the Errata sheet to  */
   /* know if a specific device is affected DO NOT ALTER THIS FUNCTION  */

   /* Open PMM registers for write access.                              */
   PMMCTL0_H     = 0xA5;

   /* Disable dedicated Interrupts and backup all registers.            */
   PMMRIE_backup    = PMMRIE;
   PMMRIE          &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE | SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
   SVSMHCTL_backup  = SVSMHCTL;
   SVSMLCTL_backup  = SVSMLCTL;

   /* Clear flags.                                                      */
   PMMIFG           = 0;

   /* Set SVM highside to new level and check if a VCore increase is    */
   /* possible.                                                         */
   SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * Level);

   /* Wait until SVM highside is settled.                               */
   while ((PMMIFG & SVSMHDLYIFG) == 0);

   /* Clear flag.                                                       */
   PMMIFG &= ~SVSMHDLYIFG;

   /* Check if a VCore increase is possible.                            */
   if((PMMIFG & SVMHIFG) == SVMHIFG)
   {
      /* Vcc is too low for a Vcore increase so we will recover the     */
      /* previous settings                                              */
   	PMMIFG &= ~SVSMHDLYIFG;
   	SVSMHCTL = SVSMHCTL_backup;

      /* Wait until SVM highside is settled.                            */
   	while ((PMMIFG & SVSMHDLYIFG) == 0)
         ;

      /* Return that the value was not set.                             */
      Result = 1;
   }
   else
   {
      /* Set also SVS highside to new level Vcc is high enough for a    */
      /* Vcore increase                                                 */
      SVSMHCTL |= (SVSHRVL0 * Level);

      /* Wait until SVM highside is settled.                            */
      while ((PMMIFG & SVSMHDLYIFG) == 0)
         ;

      /* Clear flags.                                                   */
      PMMIFG &= ~SVSMHDLYIFG;

      /* Set VCore to new level.                                        */
      PMMCTL0_L = PMMCOREV0 * Level;

      /* Set SVM, SVS low side to new level.                            */
      SVSMLCTL = SVMLE | (SVSMLRRL0 * Level) | SVSLE | (SVSLRVL0 * Level);

      /* Wait until SVM, SVS low side is settled.                       */
      while ((PMMIFG & SVSMLDLYIFG) == 0)
         ;

      /* Clear flag.                                                    */
      PMMIFG &= ~SVSMLDLYIFG;

      /* SVS, SVM core and high side are now set to protect for the new */
      /* core level.  Restore Low side settings Clear all other bits    */
      /* _except_ level settings                                        */
      SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

      /* Clear level settings in the backup register,keep all other     */
      /* bits.                                                          */
      SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

      /* Restore low-side SVS monitor settings.                         */
      SVSMLCTL |= SVSMLCTL_backup;

      /* Restore High side settings.  Clear all other bits except level */
      /* settings                                                       */
      SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

      /* Clear level settings in the backup register,keep all other     */
      /* bits.                                                          */
      SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

      /* Restore backup.                                                */
      SVSMHCTL |= SVSMHCTL_backup;

      /* Wait until high side, low side settled.                        */
      while(((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0))
         ;

      /* Return that the value was set.                                 */
      Result = 0;
   }

   /* Clear all Flags.                                                  */
   PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

   /* Restore PMM interrupt enable register.                            */
   PMMRIE = PMMRIE_backup;

   /* Lock PMM registers for write access.                              */
   PMMCTL0_H = 0x00;

   return(Result);
}

   /* The following function is a utility function the is used to       */
   /* decrement the VCore setting to the specified value.               */
static unsigned char DecrementVCORE(unsigned char Level)
{
   unsigned char Result;
   unsigned char PMMRIE_backup;
   unsigned char SVSMHCTL_backup;
   unsigned char SVSMLCTL_backup;

   /* The code flow for decreasing the Vcore has been altered to work   */
   /* around the erratum FLASH37.  Please refer to the Errata sheet to  */
   /* know if a specific device is affected DO NOT ALTER THIS FUNCTION  */

   /* Open PMM registers for write access.                              */
   PMMCTL0_H        = 0xA5;

   /* Disable dedicated Interrupts Backup all registers                 */
   PMMRIE_backup    = PMMRIE;
   PMMRIE          &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE | SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
   SVSMHCTL_backup  = SVSMHCTL;
   SVSMLCTL_backup  = SVSMLCTL;

   /* Clear flags.                                                      */
   PMMIFG &= ~(SVMHIFG | SVSMHDLYIFG | SVMLIFG | SVSMLDLYIFG);

   /* Set SVM, SVS high & low side to new settings in normal mode.      */
   SVSMHCTL = SVMHE | (SVSMHRRL0 * Level) | SVSHE | (SVSHRVL0 * Level);
   SVSMLCTL = SVMLE | (SVSMLRRL0 * Level) | SVSLE | (SVSLRVL0 * Level);

   /* Wait until SVM high side and SVM low side is settled.             */
   while (((PMMIFG & SVSMHDLYIFG) == 0) || ((PMMIFG & SVSMLDLYIFG) == 0))
      ;

   /* Clear flags.                                                      */
   PMMIFG &= ~(SVSMHDLYIFG + SVSMLDLYIFG);

   /* SVS, SVM core and high side are now set to protect for the new    */
   /* core level.                                                       */

   /* Set VCore to new level.                                           */
   PMMCTL0_L = PMMCOREV0 * Level;

   /* Restore Low side settings Clear all other bits _except_ level     */
   /* settings                                                          */
   SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

   /* Clear level settings in the backup register,keep all other bits.  */
   SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

   /* Restore low-side SVS monitor settings.                            */
   SVSMLCTL |= SVSMLCTL_backup;

   /* Restore High side settings Clear all other bits except level      */
   /* settings                                                          */
   SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

   /* Clear level settings in the backup register, keep all other bits. */
   SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

   /* Restore backup.                                                   */
   SVSMHCTL |= SVSMHCTL_backup;

   /* Wait until high side, low side settled.                           */
   while (((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0))
      ;

   /* Clear all Flags.                                                  */
   PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

   /* Restore PMM interrupt enable register.                            */
   PMMRIE = PMMRIE_backup;

   /* Lock PMM registers for write access.                              */
   PMMCTL0_H = 0x00;

   /* Return success to the caller.                                     */
   Result    = 0;

   return(Result);
}

   /* The following function is responsible for setting the PMM core    */
   /* voltage to the specified level.                                   */
static void ConfigureVCore(unsigned char Level)
{
   unsigned int ActualLevel;
   unsigned int Status;

   /* Set Mask for Max.  level.                                         */
   Level       &= PMMCOREV_3;

   /* Get actual VCore.                                                 */
   ActualLevel  = (PMMCTL0 & PMMCOREV_3);

   /* Step by step increase or decrease the VCore setting.              */
   Status = 0;
   while (((Level != ActualLevel) && (Status == 0)) || (Level < ActualLevel))
   {
     if (Level > ActualLevel)
       Status = IncrementVCORE(++ActualLevel);
     else
       Status = DecrementVCORE(--ActualLevel);
   }
}

/*---------------------------------------------------------------------------*/
static void
init_ports(void)
{
  /* Turn everything off, device drivers enable what is needed. */

  /* All configured for digital I/O */
#ifdef P1SEL
  P1SEL = 0;
#endif
#ifdef P2SEL
  P2SEL = 0;
#endif
#ifdef P3SEL
  P3SEL = 0;
#endif
#ifdef P4SEL
  P4SEL = 0;
#endif
#ifdef P5SEL
  P5SEL = 0;
#endif
#ifdef P6SEL
  P6SEL = 0;
#endif

  /* All available inputs */
#ifdef P1DIR
  P1DIR = 0;
  P1OUT = 0;
#endif
#ifdef P2DIR
  P2DIR = 0;
  P2OUT = 0;
#endif
#ifdef P3DIR
  P3DIR = 0;
  P3OUT = 0;
#endif
#ifdef P4DIR
  P4DIR = 0;
  P4OUT = 0;
#endif

#ifdef P5DIR
  P5DIR = 0;
  P5OUT = 0;
#endif

#ifdef P6DIR
  P6DIR = 0;
  P6OUT = 0;
#endif

#ifdef P7DIR
  P7DIR = 0;
  P7OUT = 0;
  P7SEL |= BIT0 | BIT1;     /* Configure for ext clock function on these pins */
#endif

#ifdef P8DIR
  P8DIR = 0;
  P8OUT = 0;
#endif

  P1IE = 0;
  P2IE = 0;
}
/*---------------------------------------------------------------------------*/
/* msp430-ld may align _end incorrectly. Workaround in cpu_init. */
#ifndef __IAR_SYSTEMS_ICC__
extern int _end;                /* Not in sys/unistd.h */
static char *cur_break = (char *)&_end;
#endif


   /* The following function is responsible for starting XT1 in the     */
   /* MSP430 that is used to source the internal FLL that drives the    */
   /* MCLK and SMCLK.                                                   */
static void StartCrystalOscillator(void)
{
   /* Set up XT1 Pins to analog function, and to lowest drive           */
   P7SEL |= (BIT1 | BIT0);

   /* Set internal cap values.                                          */
   UCSCTL6 |= XCAP_3;

   /* Loop while the Oscillator Fault bit is set.                       */
   while(SFRIFG1 & OFIFG)
   {
     while (SFRIFG1 & OFIFG)
     {
        /* Clear OSC fault flags.                                       */
       UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT1HFOFFG + XT2OFFG);
       SFRIFG1 &= ~OFIFG;
     }

     /* Reduce the drive strength.                                      */
     UCSCTL6 &= ~(XT1DRIVE1_L + XT1DRIVE0);
   }
}

const unsigned char VCORE_Level = PMMCOREV_1;
const unsigned int DCO_Multiplier = 488;

#if 0
  {PMMCOREV_0, 244},  /* cf8MHZ_t.                          */
   {PMMCOREV_1, 488},  /* cf16MHZ_t.                         */
   {PMMCOREV_2, 610},  /* cf20MHZ_t.                         */
   {PMMCOREV_3, 762}   /* cf25MHZ_t.                         */
#endif

   /* The following function is responsible for setting up the system   */
   /* clock at a specified frequency.                                   */
static void SetSystemClock()
{
   unsigned char                   UseDCO;
   unsigned int                    Ratio;
   unsigned int                    DCODivBits;
   unsigned long                   SystemFrequency;
   volatile unsigned int           Counter;

   /* Configure the PMM core voltage.                                   */
   ConfigureVCore(VCORE_Level);

   /* Get the ratio of the system frequency to the source clock.        */
   Ratio           = DCO_Multiplier;

   /* Use a divider of at least 2 in the FLL control loop.              */
   DCODivBits      = FLLD__2;

   /* Get the system frequency that is configured.                      */
   SystemFrequency  = DCO_Multiplier * 32768L;
   SystemFrequency /= 1000;

   /* If the requested frequency is above 16MHz we will use DCO as the  */
   /* source of the system clocks, otherwise we will use DCOCLKDIV.     */
   if(SystemFrequency > 16000)
   {
       Ratio  >>= 1;
       UseDCO   = 1;
   }
   else
   {
       SystemFrequency <<= 1;
       UseDCO            = 0;
   }

   /* While needed set next higher div level.                           */
   while (Ratio > 512)
   {
       DCODivBits   = DCODivBits + FLLD0;
       Ratio      >>= 1;
   }

   /* Disable the FLL.                                                  */
   __bis_SR_register(SCG0);

   /* Set DCO to lowest Tap.                                            */
   UCSCTL0 = 0x0000;

   /* Reset FN bits.                                                    */
   UCSCTL2 &= ~(0x03FF);
   UCSCTL2  = (DCODivBits | (Ratio - 1));

   /* Set the DCO Range.                                                */
   if(SystemFrequency <= 630)
   {
      /* Fsystem < 630KHz.                                              */
      UCSCTL1 = DCORSEL_0;
   }
   else if(SystemFrequency <  1250)
   {
      /* 0.63MHz < fsystem < 1.25MHz.                                   */
      UCSCTL1 = DCORSEL_1;
   }
   else if(SystemFrequency <  2500)
   {
      /* 1.25MHz < fsystem < 2.5MHz.                                    */
      UCSCTL1 = DCORSEL_2;
   }
   else if(SystemFrequency <  5000)
   {
      /* 2.5MHz < fsystem < 5MHz.                                       */
      UCSCTL1 = DCORSEL_3;
   }
   else if(SystemFrequency <  10000)
   {
      /* 5MHz < fsystem < 10MHz.                                        */
      UCSCTL1 = DCORSEL_4;
   }
   else if(SystemFrequency <  20000)
   {
      /* 10MHz < fsystem < 20MHz.                                       */
      UCSCTL1 = DCORSEL_5;
   }
   else if(SystemFrequency <  40000)
   {
      /* 20MHz < fsystem < 40MHz.                                       */
      UCSCTL1 = DCORSEL_6;
   }
   else
      UCSCTL1 = DCORSEL_7;

   /* Re-enable the FLL.                                                */
   __bic_SR_register(SCG0);

   /* Loop until the DCO is stabilized.                                 */
   while(UCSCTL7 & DCOFFG)
   {
       /* Clear DCO Fault Flag.                                         */
       UCSCTL7 &= ~DCOFFG;

       /* Clear OFIFG fault flag.                                       */
       SFRIFG1 &= ~OFIFG;
   }

   /* Enable the FLL control loop.                                      */
   __bic_SR_register(SCG0);

   /* Based on the frequency we will use either DCO or DCOCLKDIV as the */
   /* source of MCLK and SMCLK.                                         */
   if (UseDCO)
   {
      /* Select DCOCLK for MCLK and SMCLK.                              */
      UCSCTL4 &=  ~(SELM_7 | SELS_7);
      UCSCTL4 |= (SELM__DCOCLK | SELS__DCOCLK);
   }
   else
   {
      /* Select DCOCLKDIV for MCLK and SMCLK.                           */
       UCSCTL4 &=  ~(SELM_7 | SELS_7);
       UCSCTL4 |= (SELM__DCOCLKDIV | SELS__DCOCLKDIV);
   }

   /* Delay the appropriate amount of cycles for the clock to settle.   */
   Counter = Ratio * 32;
   while (Counter--)
       __delay_cycles(30);
}


void
msp430_cpu_init(void)
{
  dint();
  init_ports();

  /* Call the MSP430F5438 Experimentor Board Hardware Abstraction Layer*/
  /* to setup the system clock.                                        */
  StartCrystalOscillator();
  SetSystemClock();

  eint();
#ifndef __IAR_SYSTEMS_ICC__
  if((uintptr_t)cur_break & 1) { /* Workaround for msp430-ld bug! */
    cur_break++;
  }
#endif
}
