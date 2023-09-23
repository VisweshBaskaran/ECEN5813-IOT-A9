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
#include "src/oscillators.h"
#include "math.h"
#include "app.h"

// Macros
/*  ULFRCO freq = 1000Hz
 *  1 counter tick = 1ms
 *  Hence least valid input for timerwaitUS = 1000us = 1ms
 * */
#define EM3_MIN_US_VAL 1000
/*
 * 4 / 32768Hz = 0.12207 ms = 122.07us
 * 1 counter tick = 123 us
 * Hence least valid input for timerwaitUs = 123
 */
#define EM_0TO2_MIN_US_VAL 123
//As per requirement
#define MAX_US_VAL 8e6

#define LFXO_PRESCALER_VALUE 4
#define ULFRCO_PRESCALER_VALUE 1

#define LETIMER_PERIOD_MS (3000)

#if LOWEST_ENERGY_MODE == 3
#define ACTUAL_CLK_FREQ 1000/ULFRCO_PRESCALER_VALUE
#elif LOWEST_ENERGY_MODE !=3
#define ACTUAL_CLK_FREQ 32768/LFXO_PRESCALER_VALUE
#endif

#define COMP0_LOAD ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)

/*
 * @brief Initializes the LETIMER0 peripheral with the specified configuration.
 *
 * @param none
 *
 * @return none
 */
void letimer0Init(void);

/*
 * @brief Function to create delay in order of microseconds using LETIMER.
 *
 * @param us The duration to wait in microseconds.
 *
 * @return none
 */
void timerwaitUs(uint32_t us);
#endif /* SRC_TIMERS_H_ */
