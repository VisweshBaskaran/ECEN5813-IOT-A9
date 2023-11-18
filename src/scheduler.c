/**
 * File name: scheduler.c
 * File description: This file defines the APIs that schedules the interrupts according to required priority scheme
 * Date created: 21-Sep-2023
 * Updates:
 *         23-Oct-2023 Added discovery state machine for BLE client functionality
 *         27-Oct-2023, Added PB0 set event functions
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *    [1] ECEN5823 IOT Embedded Firmware lecture slides
 *    [2] Health Thermometer Characteristic and Descriptor descriptions: https://www.bluetooth.com/specifications/assigned-numbers/
 */

#include "src/scheduler.h"


#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


typedef enum
{
  IDLE,
  WAIT_FOR_STABILIZE,
  I2C_WRITE,
  WAIT_FOR_CONVERSION,
  I2C_READ,
} Server_State_t; //States for temperature state machine

typedef enum
{
  IDLE_CLIENT,
  DISCOVER_BUTTON_SERVICE,
  SERVICES_DISCOVERED,
  DISCOVER_BUTTON_CHARACTERISTICS,
  CHARACTERISTICS_DISCOVERED,
  SET_BUTTON_INDICATIONS,
  INDICATION_ENABLED,
  WAIT_FOR_CLOSE
}Client_State_t; //States for discovery state machine



uint32_t myEvents = CLEAR_EVENT;

/**
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

/**
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

/**
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

/**
 *  @brief Sets PB0 pressed flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB0Pressed(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB0_pressed);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

/**
 *  @brief Sets PB0 released flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB0Released(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB0_released);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

/**
 *  @brief Sets PB1 pressed flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB1Pressed(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB1_pressed);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

/**
 *  @brief Sets PB1 released flag in the scheduler
 *
 *  @param none
 *
 *  @return none
 */
void schedulerSetEventPB1Released(void)
{
  CORE_DECLARE_IRQ_STATE;
  // set event
  CORE_ENTER_CRITICAL(); // enter critical, turn off interrupts in NVIC
  sl_bt_external_signal(evtPB1_released);
  CORE_EXIT_CRITICAL(); // exit critical, re-enable interrupts in NVIC
}

/**
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

  if(myEvents & evtPB0_pressed)
    {
      //Selecting 1 event to return to main() code with priorities applied
      theEvent = evtPB0_pressed;
      //Clearing the event
      myEvents &= ~evtPB0_pressed;

    }
  if(myEvents & evtPB0_released)
    {
      //Selecting 1 event to return to main() code with priorities applied
      theEvent = evtPB0_released;
      //Clearing the event
      myEvents &= ~evtPB0_released;

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
#if (DEVICE_IS_BLE_SERVER == 1)
void temperature_state_machine(sl_bt_msg_t *evt)
{
  Server_State_t currentState;
  static Server_State_t nextState = IDLE;
  currentState = nextState;
  if((SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_external_signal_id)) //removed double check for connection is open and ok_to_send indications are true from A5
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
              //si7021SetOn(); /*Enabled in displayInit() for A6*
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
              //si7021SetOff();
              ble_write_temp_from_si7021();
            }
          break;
      }
    }
}

#else
/**
 * @brief This function implements a state machine to handle BLE service discovery
 *
 * @param evt Pointer to the Bluetooth event message.
 *
 * @returns none
 */
void discovery_state_machine(sl_bt_msg_t *evt)
{
  ble_data_struct_t *ble_data_ptr     = get_ble_data_ptr();

         Client_State_t currentState;
  static Client_State_t nextState     = IDLE_CLIENT;

  sl_status_t sc = SL_STATUS_OK;


  currentState = nextState;

  switch(currentState)
  {
    case IDLE_CLIENT:
      //DOS - No this was done in open_id event!!! ble_data_ptr->connection_handle = evt->data.evt_connection_opened.connection;
      nextState = IDLE_CLIENT;  //default state

      // Check if a connection has been opened.
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_opened_id)
        {
          uint8_t ServiceUUID[2] = {0x09,0x18};
          // Discover primary services with health thermometer service UUID.
          sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data_ptr->connection_handle,
                                                            sizeof(ServiceUUID),
                                                            (const uint8_t*)ServiceUUID);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          nextState = DISCOVER_BUTTON_SERVICE;
        }
      break;

    case DISCOVER_BUTTON_SERVICE:
      nextState = DISCOVER_BUTTON_SERVICE;  //default state

      // Check if a GATT procedure has been completed (discover htm service)
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          uint8_t Button_ServiceUUID[16] = {0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00};
          // Discover primary services with button service UUID.
          sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data_ptr->connection_handle,
                                                            sizeof(Button_ServiceUUID),
                                                            (const uint8_t*)Button_ServiceUUID);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_primary_services_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          nextState = SERVICES_DISCOVERED;
        }
      break;


    case SERVICES_DISCOVERED:
      nextState = SERVICES_DISCOVERED;  //default state
      // Check if a GATT procedure has been completed (service discovery in this case).
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          uint8_t CharacteristicUUID[2] = {0x1c, 0x2a}; //[2]
          // Discover characteristics for health thermometer UUID within the previously discovered service.
          sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data_ptr->connection_handle,
                                                           ble_data_ptr->htm_service_handle,
                                                           sizeof(CharacteristicUUID),
                                                           (const uint8_t*)CharacteristicUUID);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          nextState = DISCOVER_BUTTON_CHARACTERISTICS;
        }
      break;

    case DISCOVER_BUTTON_CHARACTERISTICS:
      nextState = DISCOVER_BUTTON_CHARACTERISTICS;  //default state


      // Check if a GATT procedure has been completed (discover htm characteristics)
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          uint8_t Button_CharacteristicUUID[16] = {0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00};
          // Discover primary services with button service UUID.
          sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data_ptr->connection_handle,
                                                           ble_data_ptr->button_service_handle,
                                                           sizeof(Button_CharacteristicUUID),
                                                           (const uint8_t*)Button_CharacteristicUUID);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_discover_characteristics_by_uuid() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
          nextState = CHARACTERISTICS_DISCOVERED;
        }
      break;

    case CHARACTERISTICS_DISCOVERED:
      nextState = CHARACTERISTICS_DISCOVERED;  //default state
      // Check if a GATT procedure has been completed (button characteristic discovery in this case).
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          // Enable indications for HTM char.
          //LOG_INFO("Enabling HTM indications");
          sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connection_handle,
                                                          ble_data_ptr->htm_characteristic_handle,
                                                          sl_bt_gatt_indication); // sl_bt_gatt_disable sl_bt_gatt_indication
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
          }
          nextState = SET_BUTTON_INDICATIONS;
        }
      break;

    case SET_BUTTON_INDICATIONS:
      nextState = SET_BUTTON_INDICATIONS; //default state
      // Check if a GATT procedure has been completed (send htm indications).
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          // Enable indications for button_state char.
          //LOG_INFO("Enabling BTN indications");
          sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connection_handle,
                                                          ble_data_ptr->button_characteristic_handle,
                                                          sl_bt_gatt_indication);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
          }
          nextState = INDICATION_ENABLED;
        }
      break;

    case INDICATION_ENABLED:
      nextState = INDICATION_ENABLED;  //default state
      // Check if a GATT procedure has been completed (indication setup in this case).
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
        {
          nextState = WAIT_FOR_CLOSE;
          displayPrintf(DISPLAY_ROW_CONNECTION, "Handling indications");
        }
      break;

    case WAIT_FOR_CLOSE:
      nextState = WAIT_FOR_CLOSE; //default state
      // Check if the connection has been closed.
      if(SL_BT_MSG_ID(evt->header) == sl_bt_evt_connection_closed_id)
        nextState = IDLE_CLIENT;
      break;

  } // switch

} // discovery_state_machine()

#endif




