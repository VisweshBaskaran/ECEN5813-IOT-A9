/*
 * oscillator.c
 *
 *  Created on: 14-Sep-2023
 *      Author: Viswesh
 */
#include "oscillator.h"
#include "em_cmu.h"


void init_osc(void)
{
  if (LOWEST_ENERGY_MODE !=3)
    {

      CMU_OscillatorEnable(cmuOsc_LFXO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
      CMU_ClockDivSet(cmuClock_LFA, cmuClkDiv_4 );
      CMU_ClockEnable(cmuClock_LFA,true);
    }
  else
    {
      CMU_OscillatorEnable(cmuOsc_ULFRCO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
      CMU_ClockDivSet(cmuClock_LFA, cmuClkDiv_1 );
      CMU_ClockEnable(cmuClock_LFA,true);
    }
}
