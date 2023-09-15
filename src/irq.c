/*
 * irq.c
 *
 *  Created on: 14-Sep-2023
 *      Author: Viswesh
 */

#include "irq.h"

void LETIMER0_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;


  CORE_ENTER_CRITICAL();

  uint32_t flag;
  flag = LETIMER_IntGetEnabled(LETIMER0);
  LETIMER_IntClear(LETIMER0,flag);

  CORE_EXIT_CRITICAL();
  if(flag & LETIMER_IF_UF)
    {
      gpioLed0SetOff();
    }
  else if (flag & LETIMER_IF_COMP1)
    {
      gpioLed0SetOn();
          }

}

