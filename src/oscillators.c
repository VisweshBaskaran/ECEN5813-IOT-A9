/*
 * File name: oscillator.c
 * File description: This file defines the API that initializes oscillator peripherals (LFXO and ULFRCO)
 * Date: 14-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 *    [2] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */
#include <src/oscillators.h>
#include "em_cmu.h"

/*
 * @brief This function configures the oscillator and clock settings according to the energy mode set.
 *
 * @param none
 *
 * @return none
 */
void oscInit(void)
{
  if(LOWEST_ENERGY_MODE != 3)
    {
      CMU_OscillatorEnable(cmuOsc_LFXO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
      CMU_ClockEnable(cmuClock_LFA,true);
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
      CMU_ClockEnable(cmuClock_LETIMER0,true);

    }
  else
    {
      CMU_OscillatorEnable(cmuOsc_ULFRCO,true, true);
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
      CMU_ClockEnable(cmuClock_LFA,true);
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);
      CMU_ClockEnable(cmuClock_LETIMER0,true);

    }
} //init_osc()

