/*
 * oscillator.h
 *
 *  Created on: 14-Sep-2023
 *      Author: Viswesh
 */

#ifndef SRC_OSCILLATOR_H_
#define SRC_OSCILLATOR_H_

#define LOWEST_ENERGY_MODE (0)
//#define LOWEST_ENERGY_MODE (1)
//#define LOWEST_ENERGY_MODE (2)
//#define LOWEST_ENERGY_MODE (3)

#define LETIMER_ON_TIME_MS (175)
#define LETIMER_PERIOD_MS (2250)

#define LXFO (32768)
#define PRESCALER_LXFO (4)

#define ULFRCO (1000)
#define PRESCALER_ULFRCO (1)


void init_osc(void);
#endif /* SRC_OSCILLATOR_H_ */
