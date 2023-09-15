/*
 * timer.h
 *
 *  Created on: 14-Sep-2023
 *      Author: Viswesh
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "em_letimer.h"
#include "oscillator.h"


#define PRESCALER_VALUE 4
#define ACTUAL_CLK_FREQ 32768/PRESCALER_VALUE
#define ACTUAL_CLK_PERIOD 1/ACTUAL_CLK_FREQ
#define LETIMER_PERIOD_MS 2250
#define COMP0_LOAD ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)
#define COMP1_LOAD ((LETIMER_ON_TIME_MS*ACTUAL_CLK_FREQ)/1000)


void init_LETIMER0(void);
#endif /* SRC_TIMER_H_ */
