/*
 * File name: scheduler.c
 * File description: This file defines the APIs that schedules the interrupts according to required priority scheme
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides weeks 3-4
 */
#include "src/scheduler.h"
typedef enum
{
  IDLE,
  WAIT_FOR_STABILIZE,
  I2C_WRITE,
  WAIT_FOR_CONVERSION,
  I2C_READ
} State_t;

uint32_t myEvents = 0;

/*
 * @brief Sets the LETIMER0 underflow event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventUF(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  myEvents |= evtLETIMER0_UF;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC

}

/*
 * @brief Sets the LETIMER0 COMP1 event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventCOMP1(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  myEvents |= evtLETIMER0_COMP1;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC

}

/*
 * @brief Sets I2C Transfer complete flag in the scheduler
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventTransferComplete(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  myEvents |= evtI2C_Transfer_Complete;
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC

}

/*
 * @brief Retrieves the next pending event and clears the event
 *
 * @param none
 *
 * @return none
 */
uint32_t getNextEvent(void)
{
  int32_t theEvent = CLEAR_EVENT; //Setting to 0 to avoid garbage value
  //theEvent =  myEvents; // 1 event to return to the caller
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  if(myEvents & evtLETIMER0_UF)
    {
      //Selecting 1 event to return to main() code with priorities applied
      theEvent = evtLETIMER0_UF;
      //Clearing the event
      myEvents &= ~evtLETIMER0_UF;

    }
  if(myEvents & evtI2C_Transfer_Complete)
    {
      //Selecting 1 event to return to main() code with priorities applied
      theEvent = evtI2C_Transfer_Complete;
      //Clearing the event
      myEvents &= ~evtI2C_Transfer_Complete;

    }
  if(myEvents & evtLETIMER0_COMP1)
    {
      //Selecting 1 event to return to main() code with priorities applied
      theEvent = evtLETIMER0_COMP1;
      //Clearing the event
      myEvents &= ~evtLETIMER0_COMP1;

    }

  CORE_EXIT_CRITICAL();
  return (theEvent);
} // getNextEvent()

/*
 * @brief State machine to read temperature using SI7021 through I2C communications
 *
 * @param evt, scheduler events to drive states
 *
 * @returns none
 */
void temperature_state_machine(uint32_t evt)
{
  State_t currentState;
  static State_t nextState = IDLE;
  currentState = nextState;
  switch(currentState)
  {
    case IDLE:
      nextState = IDLE; //default
      if(evt == evtLETIMER0_UF)
        {
          nextState = WAIT_FOR_STABILIZE;
          si7021SetOn();
          timerwaitUs_interrupt(80000);
        }
      break;
    case WAIT_FOR_STABILIZE:
      nextState = WAIT_FOR_STABILIZE; //default
      if(evt == evtLETIMER0_COMP1)
        {
          nextState = I2C_WRITE;
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          Write_I2C(0xF3);

        }
      break;
    case I2C_WRITE:
      nextState = I2C_WRITE; //default
      if(evt == evtI2C_Transfer_Complete)
        {
          nextState = WAIT_FOR_CONVERSION;
          NVIC_DisableIRQ(I2C0_IRQn);
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          timerwaitUs_interrupt(10800);
        }
      break;
    case WAIT_FOR_CONVERSION:
      nextState = WAIT_FOR_CONVERSION; //default
      if(evt == evtLETIMER0_COMP1)
        {
          nextState = I2C_READ;
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          Read_I2C();
        }
      break;
    case I2C_READ:
      nextState = I2C_READ; //default
      if(evt == evtI2C_Transfer_Complete)
        {
          nextState = IDLE;
          NVIC_DisableIRQ(I2C0_IRQn);
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
          si7021SetOff();
          read_temp_from_si7021();
        }
      break;
  }

}



