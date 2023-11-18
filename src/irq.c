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
int rollover_count = 0;

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

/*
 * @brief Interrupt handler for I2C0 communication, initiating transfers, and handling completion error
 *
 * @param none
 *
 * @returns none
 */
void I2C0_IRQHandler(void)
{
  I2C_TransferReturn_TypeDef transferStatus;
  transferStatus = I2C_Transfer(I2C0);

  if (transferStatus == i2cTransferDone)
    {
      schedulerSetEventTransferComplete();
    }
  if (transferStatus < 0)
    {
      LOG_ERROR("%d", transferStatus);
    }
}

/**
 * @brief GPIO Even Interrupt Handler: It handles button press and release events for PB0.
 *
 * @param none
 *
 * @returns none
 *
 * @reference Written with the help of ChatGPT using step 2 as prompt from Assignment 8-Approach/Guidance section
 */
void GPIO_EVEN_IRQHandler(void)
{
  uint32_t flag;
  //DOS ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();
  //flag = GPIO_IntGet();
  flag = GPIO_IntGetEnabled() & 0x55555555; // mask off odd numbered bits 1,3,5... leaving 0,2,4...

  //LOG_INFO("EvenFlag=%x", flag);

  //flag = GPIO_IntGet();
  // DOS bit[6] is PB0 and bit[7] is PB1
  ///flag = GPIO_IntGetEnabled() & 0x55555555; // mask off odd numbered bits 1,3,5... leaving 0,2,4...

  GPIO_IntClear(flag);

  //DOS uint8_t button_state = GPIO_PinInGet(PB0_port, PB0_pin); // DOS 0=pressed, 1=released
  uint32_t button_state = GPIO_PinInGet(PB0_port, PB0_pin); // DOS 0=pressed, 1=released

  //LOG_INFO("EBS=%x", button_state);

//  if (flag & (1 << PB0_pin))
//    {
      if (button_state)
        {
          //DOS ble_data_ptr->PB0_pressed = false; // DOS not pressed
          schedulerSetEventPB0Released();
          //LOG_INFO("PB0 release");
        }
      else
        {
          //DOS ble_data_ptr->PB0_pressed = true; // DOS is pressed
          schedulerSetEventPB0Pressed();
          //LOG_INFO("PB0 pressed");
        }
//    }
}


/**
 * @brief GPIO Odd Interrupt Handler: It handles button press and release events for PB1.
 *
 * @param none
 *
 * @returns none
 */
void GPIO_ODD_IRQHandler(void)
{
  uint32_t flag;
  //DOS ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();

  // DOS bit[6] is PB0 and bit[7] is PB1
  flag = GPIO_IntGetEnabled() & 0xAAAAAAAA; // mask off even numbered bits 0,2,4...

  //LOG_INFO("OddFlag=%x", flag);

  GPIO_IntClear(flag);

  uint32_t button_state = GPIO_PinInGet(PB1_port, PB1_pin); // DOS 0=pressed, 1=released

  //LOG_INFO("OBS=%x", button_state);

  if (flag & (1 << PB1_pin))
    {
      if (button_state)
        {
          //DOS ble_data_ptr->PB1_pressed = false; // DOS not pressed
          schedulerSetEventPB1Released();
          //LOG_INFO("PB1 release");
        }
      else
        {
          //DOS ble_data_ptr->PB1_pressed = true; // DOS is pressed
          schedulerSetEventPB1Pressed();
          //LOG_INFO("PB1 pressed");
        }
    }
}


/**
 * @brief Calculates the time in milliseconds using a LETIMER peripheral
 *
 * @param none
 *
 * @returns time elapsed since execution
 *
 * NOTE: Helped Aditi Vijay Nanaware with her letimerMilliseconds by providing my return statement
 */
uint32_t letimerMilliseconds(void)
{
  return (rollover_count + ((LETIMER_CompareGet(LETIMER0, 0) - LETIMER_CounterGet(LETIMER0))/LETIMER_CompareGet(LETIMER0, 0)))*LETIMER_PERIOD_MS;
}
