/*
 *  File name: irq.c
 *  File Description: This file defines the LETIMER0_IRQHandler that drives the LED based on the COMP1 and UF bits
 *  Date: 14-Sep-2023
 *  Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 *  Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 */

#include "irq.h"

/*
 * @brief Interrupt service routine for LETIMER0 peripheral to drive LED0 based on interrupt flags of LETIMER0; COMP1 and UF.
 *
 * @param none
 *
 * @returns none
 */
void LETIMER0_IRQHandler(void)
{
  //Variable to store current NVIC status
  CORE_DECLARE_IRQ_STATE;

  //Entering critical section
  CORE_ENTER_CRITICAL();
  uint32_t flag;
  flag = LETIMER_IntGetEnabled(LETIMER0);
  LETIMER_IntClear(LETIMER0,flag);

  //Exiting critical section
  CORE_EXIT_CRITICAL();

  schedulerSetEventUF();
}

