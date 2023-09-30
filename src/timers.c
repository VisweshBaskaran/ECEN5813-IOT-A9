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
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
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
  // this data structure is passed to LETIMER_Init (), used to set LETIMER0_CTRL reg bits and other values
  const LETIMER_Init_TypeDef letimerInitData =
      {
          false, // enable; don't enable when init completes, we'll enable last
          false, // debugRun; useful to have the timer running when single-stepping in the debugger
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
  //LETIMER_CompareSet(LETIMER0, 1, COMP1_LOAD);
  // Clear all IRQ flags in the LETIMER0 IF status register
  LETIMER_IntClear (LETIMER0, 0xFFFFFFFF); // punch them all down
  LETIMER_IntEnable (LETIMER0, LETIMER_IEN_UF); // Make sure you have defined the ISR routine LETIMER0_IRQHandler()
  // Enable the timer to starting counting down, set LETIMER0_CMD[START] bit, see LETIMER0_STATUS[RUNNING] bit
  LETIMER_Enable (LETIMER0, true);
} //letimer0Init()

/*
 * @brief Function to create delay in order of microseconds using LETIMER.
 *
 * @param us The duration to wait in microseconds.
 *
 * @return none
 */
void timerwaitUs_polled(uint32_t us)
{
  // Ensuring the requested delay is within a valid range and clamp if necessary.
  if (us > MAX_US_VAL)
    {
      LOG_ERROR("Requested us delay is over the range. Clamped to max value: 8e6us\r\n");
      us = MAX_US_VAL;
    }
  if(us< EM3_MIN_US_VAL)
    {
      LOG_ERROR("Requested us delay is under the range. Clamped to min value: 1000us\r\n");
      us = EM3_MIN_US_VAL;
    }
  uint32_t needed_ticks, current_tick, target_tick;
  // Calculate the number of timer ticks needed for the specified delay.
  needed_ticks = us/ULFRCO;
  current_tick = LETIMER_CounterGet(LETIMER0);
  // If time remaining is greater than current tick, count down from COMP0_LOAD.
  // Otherwise, count down from the current tick.
  target_tick  = (needed_ticks > current_tick) ? (uint32_t)(COMP0_LOAD - (needed_ticks - current_tick)) : (current_tick - needed_ticks);

  while(LETIMER_CounterGet(LETIMER0) != target_tick)
    ; //busy wait

}
void timerwaitUs_interrupt(uint32_t us)
{
  // Ensuring the requested delay is within a valid range and clamp if necessary.
  if (us > MAX_US_VAL)
    {
      LOG_ERROR("Requested us delay is over the range. Clamped to max value: 8e6us\r\n");
      us = MAX_US_VAL;
    }
  if(us< EM3_MIN_US_VAL)
    {
      LOG_ERROR("Requested us delay is under the range. Clamped to min value: 1000us\r\n");
      us = EM3_MIN_US_VAL;
    }
  uint32_t needed_ticks, current_tick, target_tick;
  needed_ticks = us/ULFRCO;
  current_tick = LETIMER_CounterGet(LETIMER0);
  // If time remaining is greater than current tick, count down from COMP0_LOAD.
  // Otherwise, count down from the current tick.
  target_tick  = (needed_ticks > current_tick) ? (uint32_t)(COMP0_LOAD - (needed_ticks - current_tick)) : (current_tick - needed_ticks);

  LETIMER_IntClear (LETIMER0, 0xFFFFFFFF); // punch them all down
  LETIMER_CompareSet(LETIMER0, 1, target_tick);
  int32_t temp = LETIMER_IEN_COMP1;
  LETIMER_IntEnable(LETIMER0, temp);
  //Force loading due to ssv5 compiler bug
  LETIMER_TypeDef *letimer;
  letimer = LETIMER0;
  letimer->IEN |= LETIMER_IEN_COMP1;
}
