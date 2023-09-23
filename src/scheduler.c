/*
 * File name: scheduler.c
 * File description: This file defines the APIs that schedules the interrupts according to required priority scheme
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 3
 */
#include "src/scheduler.h"
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
 * @brief Retrieves the next pending event and clears the event
 *
 * @param none
 *
 * @return none
 */
uint32_t getNextEvent(void)
{
  int32_t theEvent;
  theEvent =  myEvents; // 1 event to return to the caller
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  myEvents = CLEAR_EVENT;
  CORE_EXIT_CRITICAL();
  return (theEvent);
} // getNextEvent()


