/*
 * timer.h
 *
 *  Created on: 14-Sep-2023
 *      Author: Viswesh
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "em_letimer.h"
#include "src/oscillator.h"
#include "app.h"

#define LFXO_PRESCALER_VALUE 4
#define ULFRCO_PRESCALER_VALUE 1

#if LOWEST_ENERGY_MODE == 3
#define ACTUAL_CLK_FREQ 1000/ULFRCO_PRESCALER_VALUE
#define ACTUAL_CLK_PERIOD 1/ACTUAL_CLK_FREQ
#elif LOWEST_ENERGY_MODE !=3
#define ACTUAL_CLK_FREQ 32768/LFXO_PRESCALER_VALUE
#define ACTUAL_CLK_PERIOD 1/ACTUAL_CLK_FREQ
#endif

#define COMP0_LOAD ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)
#define COMP1_LOAD ((LETIMER_ON_TIME_MS*ACTUAL_CLK_FREQ)/1000)


void init_LETIMER0(void);
#endif /* SRC_TIMER_H_ */
