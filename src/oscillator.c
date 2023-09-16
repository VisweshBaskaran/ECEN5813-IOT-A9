/*
 * File name: oscillator.c
 * File description:
 *                This file initializes the oscillator peripherals (LFXO and ULFRCO)
 * Date: 14-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 */
#include "src/oscillator.h"
#include "em_cmu.h"


void init_osc(void)
{
  if(LOWEST_ENERGY_MODE != 3)
    {
      CMU_OscillatorEnable(cmuOsc_LFXO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
      CMU_ClockDivSet(cmuClock_LFA, cmuClkDiv_4);
      CMU_ClockEnable(cmuClock_LFA,true);
    }
  else
    {
      CMU_OscillatorEnable(cmuOsc_ULFRCO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
      CMU_ClockDivSet(cmuClock_LFA, cmuClkDiv_1);
      CMU_ClockEnable(cmuClock_LFA,true);
    }
}

