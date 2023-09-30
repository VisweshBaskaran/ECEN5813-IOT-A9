/*
 *  File name: irq.h
 *  File Description: This is a header file that includes necessary libraries for irq.c
 *  Date: 14-Sep-2023
 *  Author: Visweshwaran Baskaran viba5388@colorado.edu
 *  Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 */
#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_

#include "em_core.h"
#include "src/gpio.h"
#include "src/timers.h"
#include "src/scheduler.h"

/*
 * @brief Calculates the time in milliseconds using a LETIMER peripheral
 *
 * @param none
 *
 * @returns time elapsed since execution
 */
uint32_t letimerMilliseconds(void);

/*
 * @brief Interrupt service routine for LETIMER0 peripheral to drive LED0 based on interrupt flags of LETIMER0; COMP1 and UF.
 *
 * @param none
 *
 * @returns none
 */
void LETIMER0_IRQHandler(void);

/*
 * @brief Interrupt handler for I2C0 communication, initiating transfers, and handling completion error
 *
 * @param none
 *
 * @returns none
 */
void I2C0_IRQHandler(void);

#endif /* SRC_IRQ_H_ */
