/*
 *  File name: irq.c
 *  File Description: This file defines the LETIMER0_IRQHandler that drives the LED based on the COMP1 and UF bits
 *  Date: 14-Sep-2023
 *  Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 *  Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 2-3
 */

#include "irq.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
int rollover_count;

/*
 * @brief Interrupt service routine for LETIMER0 peripheral to drive LED0 based on interrupt flags of LETIMER0; COMP1 and UF.
 *
 * @param none
 *
 * @returns none
 */
void LETIMER0_IRQHandler(void)
{

  uint32_t flag;
  flag = LETIMER_IntGetEnabled(LETIMER0);
  LETIMER_IntClear(LETIMER0,flag);

  if(flag & LETIMER_IF_COMP1)
    {
      schedulerSetEventCOMP1();
      LETIMER_IntDisable(LETIMER0,LETIMER_IEN_COMP1);
    }

  if(flag & LETIMER_IF_UF)
    {
      schedulerSetEventUF();
      CORE_DECLARE_IRQ_STATE;
      CORE_ENTER_CRITICAL();

      rollover_count++;
      CORE_EXIT_CRITICAL();
    }



}

void I2C0_IRQHandler(void)
{
  I2C_TransferReturn_TypeDef transferStatus;
  transferStatus = I2C_Transfer(I2C0);

  if (transferStatus == i2cTransferDone) {
  schedulerSetEventTransferComplete();
  }
  if (transferStatus < 0) {
  LOG_ERROR("%d", transferStatus);
  }
}

uint32_t letimerMilliseconds(void)
{
  /*
   * rollover count * 3000 + (8192 - current counter/8192) * 3000
   */
  return (rollover_count + ((ACTUAL_CLK_FREQ - LETIMER_CounterGet(LETIMER0))/ACTUAL_CLK_FREQ))*LETIMER_PERIOD_MS;
}
