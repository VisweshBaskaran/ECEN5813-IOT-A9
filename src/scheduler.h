/**
 * File name: scheduler.h
 * File description: This file declares the APIs that schedules the interrupts according to required priority scheme
 * Date created: 21-Sep-2023
 * Updates:
 *         23-Oct-2023 Added discovery state machine for BLE client functionality
 *         27-Oct-2023, Added PB0 set event functions
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5823 IOT Embedded Firmware lecture slides
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_
#include "em_core.h"
#include "app.h"
enum {
  evtLETIMER0_UF           = 0b0000001,
  evtLETIMER0_COMP1        = 0b0000010,
  evtI2C_Transfer_Complete = 0b0000100,
  evtPB0_pressed           = 0b0001000,
  evtPB0_released          = 0b0010000,
  evtPB1_pressed           = 0b0100000,
  evtPB1_released          = 0b1000000
};

#define CLEAR_EVENT 0
#define MY_STATES 5



/**
 * @brief Sets the LETIMER0 underflow event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventUF(void);

/**
 * @brief Sets the LETIMER0 COMP1 event flag in the scheduler.
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventCOMP1(void);

/**
 * @brief Sets I2C Transfer complete flag in the scheduler
 *
 * @param none
 *
 * @return none
 */
void schedulerSetEventTransferComplete(void);

/**
 *  @brief Sets PB0 pressed flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB0Pressed(void);

/**
 *  @brief Sets PB0 released flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB1Released(void);

/**
 *  @brief Sets PB1 pressed flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB1Pressed(void);

/**
 *  @brief Sets PB1 released flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB0Released(void);
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
/**
 * @brief This function implements a state machine to handle BLE service discovery
 *
 * @param evt, pointer to the Bluetooth event message.
 *
 * @returns none
 */
void discovery_state_machine(sl_bt_msg_t *evt);




#endif /* SRC_SCHEDULER_H_ */
