/*
 * File name: timer.c
 * File description: This file defines the API that configures the LETIMER0 peripheral
 * Date: 14-Sep-2023
 * Author: Dave Sluiter dave.sluiter@colorado.edu
 * Modified by: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3 (Code leveraged as is from slides)
 *    [2] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */


#include <src/timers.h>

/*
 * @brief Initializes the LETIMER0 peripheral with the specified configuration.
 *
 * @param none
 *
 * @return none
 */
void letimer0Init()
{
  int32_t temp;
  // this data structure is passed to LETIMER_Init (), used to set LETIMER0_CTRL reg bits and other values
  const LETIMER_Init_TypeDef letimerInitData =
      {
      false, // enable; don't enable when init completes, we'll enable last
      true, // debugRun; useful to have the timer running when single-stepping in the debugger
      true, // comp0Top; load COMP0 into CNT on underflow
      false, // bufTop; don't load COMP1 into COMP0 when REP0==0
      0, // out0Pol; 0 default output pin value
      0, // out1Pol; 0 default output pin value
      letimerUFOANone, // ufoa0; no underflow output action
      letimerUFOANone, // ufoa1; no underflow output action
      letimerRepeatFree, // repMode; free running mode i.e. load & go forever
      0 // COMP0(top) Value, I calculate this below
      };
  // init the timer
  LETIMER_Init (LETIMER0, &letimerInitData);
  // calculate and load COMP0 (top)
  LETIMER_CompareSet(LETIMER0, 0, COMP0_LOAD);
  // calculate and load COMP1
  LETIMER_CompareSet(LETIMER0, 1, COMP1_LOAD);
  // Clear all IRQ flags in the LETIMER0 IF status register
  LETIMER_IntClear (LETIMER0, 0xFFFFFFFF); // punch them all down
  // Set UF and COMP1 in LETIMER0_IEN, so that the timer will generate IRQs to the NVIC.
  temp = LETIMER_IEN_UF | LETIMER_IEN_COMP1;
  LETIMER_IntEnable (LETIMER0, temp); // Make sure you have defined the ISR routine LETIMER0_IRQHandler()
  // Enable the timer to starting counting down, set LETIMER0_CMD[START] bit, see LETIMER0_STATUS[RUNNING] bit
  LETIMER_Enable (LETIMER0, true);
} //letimer0Init()
