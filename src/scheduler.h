/*
 * File name: scheduler.h
 * File description: This file declares the APIs that schedules the interrupts according to required priority scheme
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides week 3
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core.h"
#include "app.h"
enum {
  evtLETIMER0_UF = 0b1
  //evt_A = 0b10,
  //evt_B = 0b100
};

#define CLEAR_EVENT 0


/*
 * @brief Sets the LETIMER0 underflow event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventUF(void);

/*
 * @brief Retrieves the next pending event and clears the event
 *
 * @param none
 *
 * @return none
 */
uint32_t getNextEvent(void);

#endif /* SRC_SCHEDULER_H_ */
