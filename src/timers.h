/*
 * File name: timer.c
 * File description: This file declares the API that configures the LETIMER0 peripheral
 * Date: 14-Sep-2023
 * Author: Dave Sluiter dave.sluiter@colorado.edu
 * Modified by: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

//Required header files
#include "em_letimer.h"
#include "em_cmu.h"
#include "oscillators.h"
#include "app.h"

// Macros
#define LFXO_PRESCALER_VALUE 4
#define ULFRCO_PRESCALER_VALUE 1

#if LOWEST_ENERGY_MODE == 3
#define ACTUAL_CLK_FREQ 1000/ULFRCO_PRESCALER_VALUE
#elif LOWEST_ENERGY_MODE !=3
#define ACTUAL_CLK_FREQ 32768/LFXO_PRESCALER_VALUE
#endif

#define COMP0_LOAD ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)
#define COMP1_LOAD ((LETIMER_ON_TIME_MS*ACTUAL_CLK_FREQ)/1000)

/*
 * @brief Initializes the LETIMER0 peripheral with the specified configuration.
 *
 * @param none
 *
 * @return none
 */
void letimer0Init(void);
#endif /* SRC_TIMERS_H_ */
