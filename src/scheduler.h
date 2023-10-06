/*
 * File name: scheduler.h
 * File description: This file declares the APIs that schedules the interrupts according to required priority scheme
 * Date: 21-Sep-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5813 IOT Embedded Firmware lecture slides weeks 3-4
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core.h"
#include "app.h"
enum {
  evtLETIMER0_UF = 0b01,
  evtLETIMER0_COMP1 = 0b10,
  evtI2C_Transfer_Complete = 0b100
};

#define CLEAR_EVENT 0
#define MY_STATES 5

/*
 * @brief Sets the LETIMER0 underflow event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventUF(void);

/*
 * @brief Sets the LETIMER0 COMP1 event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventCOMP1(void);

/*
 * @brief Sets I2C Transfer complete flag in the scheduler
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventTransferComplete(void);
/*
 * @brief Retrieves the next pending event and clears the event
 *
 * @param none
 *
 * @return none
 */
uint32_t getNextEvent(void);

/*
 * @brief State machine to read temperature using SI7021 through I2C communications
 *
 * @param evt, scheduler events to drive states
 *
 * @returns none
 */
void temperature_state_machine(sl_bt_msg_t *evt);

#endif /* SRC_SCHEDULER_H_ */
