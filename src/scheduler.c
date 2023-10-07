/*
 * File name: scheduler.c
 * File description: This file defines the APIs that schedules the interrupts according to required priority scheme
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides weeks 3-4
 */
//#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
#include "src/scheduler.h"
typedef enum
{
  IDLE,
  WAIT_FOR_STABILIZE,
  I2C_WRITE,
  WAIT_FOR_CONVERSION,
  I2C_READ
} State_t;

uint32_t myEvents = CLEAR_EVENT;

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
  sl_bt_external_signal(evtLETIMER0_UF);
  //myEvents |= evtLETIMER0_UF;
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
  sl_bt_external_signal(evtLETIMER0_COMP1);
  //myEvents |= evtLETIMER0_COMP1;
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
  sl_bt_external_signal(evtI2C_Transfer_Complete);
  //myEvents |= evtI2C_Transfer_Complete;
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
void temperature_state_machine(sl_bt_msg_t *evt)
{
  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();
  State_t currentState;
  static State_t nextState = IDLE;
  currentState = nextState;
  if((SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal_id) && (ble_data_ptr->connection_open == true) && (ble_data_ptr->ok_to_send_htm_indications == true))
    {
      switch(currentState)
      {
        case IDLE:
         // LOG_INFO("Entered Idle state\n\r");
          nextState = IDLE; //default
          // Transition to WAIT_FOR_STABILIZE when LETIMER0_UF event occurs
          if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_UF)
            {
              nextState = WAIT_FOR_STABILIZE;
              // Enable the si7021 sensor and wait for stabilization
              si7021SetOn();
              timerwaitUs_interrupt(80000);
            }
          break;
        case WAIT_FOR_STABILIZE:
         //LOG_INFO("Entered wait_statbilize state\n\r");
          nextState = WAIT_FOR_STABILIZE; //default
          // Transition to I2C_WRITE when LETIMER0_COMP1 event occurs
          if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
            {
              nextState = I2C_WRITE;
              // Add EM1 requirement and write to I2C
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              Write_I2C(0xF3);

            }
          break;
        case I2C_WRITE:
          //LOG_INFO("Entered Write state\n\r");
          nextState = I2C_WRITE; //default
          // Transition to WAIT_FOR_CONVERSION when I2C_Transfer_Complete event occurs
          if(evt->data.evt_system_external_signal.extsignals == evtI2C_Transfer_Complete)
            {
              nextState = WAIT_FOR_CONVERSION;
              // Disable I2C interrupt and remove EM1 requirement, then wait for conversion
              NVIC_DisableIRQ(I2C0_IRQn);
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              timerwaitUs_interrupt(10800);
            }
          break;
        case WAIT_FOR_CONVERSION:
         //LOG_INFO("Entered wait_conversion state\n\r");
          nextState = WAIT_FOR_CONVERSION; //default
          // Transition to I2C_READ when LETIMER0_COMP1 event occurs
          if(evt->data.evt_system_external_signal.extsignals == evtLETIMER0_COMP1)
            {
              nextState = I2C_READ;
              // Add EM1 requirement and read from I2C
              sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
              Read_I2C();
            }
          break;
        case I2C_READ:
         //LOG_INFO("Entered Read state\n\r");
          nextState = I2C_READ; //default
          // Transition to IDLE when I2C_Transfer_Complete event occurs
          if(evt->data.evt_system_external_signal.extsignals == evtI2C_Transfer_Complete)
            {
              nextState = IDLE;
              // Disable I2C interrupt, remove EM1 requirement, turn off si7021 sensor,
              // and read temperature from si7021
              NVIC_DisableIRQ(I2C0_IRQn);
              sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
              si7021SetOff();
              ble_write_temp_from_si7021();
            }
          break;
      }
    }
}



