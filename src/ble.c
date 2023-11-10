/*
 * File name: ble.c
 * File description: This file declares the BLE APIs
 * Date: 04-Oct-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *  [1] ECEN5823 IOT Embedded Firmware lecture slides week 5
 *  [2] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 *
 *  Credit:
 *  Client section of handle ble event: flag updates in each event and code under case sl_bt_evt_gatt_characteristic_value_id were received from Aditi Vijay Nanaware.
 */

#include "src/ble.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Macros
//#define LOG_CONNECTION_PARAMETERS 1

//Variables required
ble_data_struct_t ble_data_ptr; // DOS this isn't a pointer to the data, its the actual data !!!
queue_struct_t indication_queue;
uint8_t button_state[2];

uint32_t advertising_interval_max = 0x190, advertising_interval_min = 0x190; //Set the Advertising minimum and maximum to 250mS. 250/0.625 = 400 = 0x190
uint16_t connection_interval_max = 0x3c, connection_interval_min = 0x3c; //Set Connection Interval minimum and maximum to 75mS. 75/1.25 =60 = 0x3c
uint16_t peripheral_latency = 4;
uint16_t supervision_timeout = 0x53; //(peripheral_latency+1)*(connection_interval * 2) + connection interval max = 825ms, approximated to 830 for integer quotient;
uint16_t log_timeout, log_latency, log_interval;

#if DEVICE_IS_BLE_SERVER == 0
uint8_t scan_interval = 0x50;
uint8_t scan_window = 0x28;
bd_addr serverAddress = SERVER_BT_ADDRESS;

#endif

//htm temperature variables
uint8_t htm_temperature_buffer[5];
uint32_t htm_temperature_flt;
uint8_t flags = 0x00;
int32_t temperature_in_c;



/*
 @brief Calculate the next pointer value in a circular buffer
 @param ptr The current pointer value
 @return The next pointer value (wrapped around the queue)
 */
static uint32_t
nextPtr (uint32_t ptr)
{
  ptr++;
  ptr = ptr % (QUEUE_DEPTH); //Wrapping around the queue
  return ptr;
} // nextPtr()

bool write_queue (uint16_t charHandle, size_t bufferLength, uint8_t *buffer)
{
  if (nextPtr (indication_queue.wptr) != indication_queue.rptr) //Checking if queue is full
    {
      indication_queue.element[indication_queue.wptr].charHandle = charHandle;
      indication_queue.element[indication_queue.wptr].bufferLength =
          bufferLength;
      memcpy (indication_queue.element[indication_queue.wptr].buffer, buffer,
              bufferLength);
      indication_queue.wptr = nextPtr (indication_queue.wptr); // write ptr incremented to next position in queue
      return READ_SUCCESS ;
    }
  return READ_FAILURE ;
} //write_queue()
bool read_queue (uint16_t *charHandle, size_t *bufferLength, uint8_t *buffer)
{
  if (indication_queue.rptr != indication_queue.wptr) //Checking if queue is empty
    {
      *charHandle = indication_queue.element[indication_queue.rptr].charHandle;
      *bufferLength =
          indication_queue.element[indication_queue.rptr].bufferLength;
      memcpy (buffer, &indication_queue.element[indication_queue.rptr].buffer,
              indication_queue.element[indication_queue.rptr].bufferLength);

      indication_queue.rptr = nextPtr (indication_queue.rptr); // read ptr incremented to next position in queue
      return WRITE_SUCCESS ;
    }
  return WRITE_FAILURE ;
}//read_queue

uint32_t get_queue_depth (void)
{
  if (indication_queue.wptr >= indication_queue.rptr)
    return (indication_queue.wptr - indication_queue.rptr);
  return (QUEUE_DEPTH - indication_queue.rptr + indication_queue.wptr); //when wptr < rptr
}
/**
 * @brief Get a pointer to the BLE data structure.
 *
 * @param none
 *
 * @return A pointer to the BLE data structure.
 */
ble_data_struct_t* get_ble_data_ptr (void)
{
  return &ble_data_ptr;
}

/**
 * @brief Handle Bluetooth Low Energy (BLE) events, including device boot, connection management,
 * and GATT (Generic Attribute Profile) operations for both clients and servers.
 *
 * @param evt - Pointer to the BLE event message.
 *
 * @returns none
 *
 * @reference ECEN5823 Lecture 10,12 slides
 */
void handle_ble_event (sl_bt_msg_t *evt)
{
  sl_status_t sc = SL_STATUS_OK;
#if DEVICE_IS_BLE_SERVER
  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr ();
  switch (SL_BT_MSG_ID(evt->header))
    {
    // ******************************************************
    // Events common to both Servers and Clients
    // ******************************************************
    // --------------------------------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack API commands before receiving this boot event!
    // Including starting BT stack soft timers!
    // --------------------------------------------------------
    case sl_bt_evt_system_boot_id:
      //Clearing all boolean flags
      ble_data_ptr->connection_open = false; //false = closed
      ble_data_ptr->ok_to_send_htm_indications = false;
      ble_data_ptr->indication_inflight = false;
      ble_data_ptr->bonding_flag = false;

      /*
       * Read the Bluetooth identity address used by the device, which can be a public
       * or random static device address.
       */
      sc = sl_bt_system_get_identity_address (&ble_data_ptr->myAddress,
                                              &ble_data_ptr->myAddressType);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_system_get_identity_address() returned != 0 status=0x%04x\n\r",
              (unsigned int) sc);
        }
      /*
       * Create an advertising set. The handle of the created advertising set is
       * returned in response.
       */
      sc = sl_bt_advertiser_create_set (&ble_data_ptr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_advertiser_create_set() returned != 0 status=0x%04x\n\r",
              (unsigned int) sc);
        }
      /*
       * Set the advertising timing parameters of the given advertising set. This
       * setting will take effect next time that advertising is enabled.
       */
      sc = sl_bt_advertiser_set_timing (ble_data_ptr->advertisingSetHandle,
                                        advertising_interval_min,
                                        advertising_interval_max, 0, 0);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_advertiser_set_timing() returned != 0 status=0x%04x\n\r",
              (unsigned int) sc);
        }
      /*Start advertising of a given advertising set with specified discoverable and connectable modes*/
      //@reference Immediate line of code based on soc_thermometer example app.c line 157-160
      sc = sl_bt_advertiser_start (ble_data_ptr->advertisingSetHandle,
                                   sl_bt_advertiser_general_discoverable,
                                   sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r",
                    (unsigned int) sc);
        }

      /*
       *  Configure security requirements and I/O capabilities of the system.
       */
      sc = sl_bt_sm_configure (0x0F, sm_io_capability_displayyesno); // MITM protection, Display with Yes/No-buttons
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_sm_configure() returned != 0 status=0x%04x",
                    (unsigned int) sc);
        }
      sc = sl_bt_sm_delete_bondings ();
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_sm_delete_bondings() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }

      displayPrintf (DISPLAY_ROW_NAME, "Server");
      displayPrintf (DISPLAY_ROW_BTADDR, "%02x:%02x:%02x:%02x:%02x:%02x",
                     ble_data_ptr->myAddress.addr[0],
                     ble_data_ptr->myAddress.addr[1],
                     ble_data_ptr->myAddress.addr[2],
                     ble_data_ptr->myAddress.addr[3],
                     ble_data_ptr->myAddress.addr[4],
                     ble_data_ptr->myAddress.addr[5]);
      displayPrintf (DISPLAY_ROW_CONNECTION, "Advertising");
      displayPrintf (DISPLAY_ROW_ASSIGNMENT, "A8");

      break;
      //This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      /* @reference
       Immediate line of code generated using ChatGPT with the following prompt:
       "After event: sl_bt_evt_connection_opened_id
       sl_bt_advertiser_stop() is called,
       Step to do: Save this connection handle. Use this value in all subsequent calls that require a connect handle, when this connection is open to the client"
       */
      ble_data_ptr->connection_handle = evt->data.evt_connection_opened.connection; // Save the connection handle for future use
      ble_data_ptr->connection_open = true;
      /*
       * Stop the advertising of the given advertising set. Counterpart with @ref
       * sl_bt_advertiser_start.
       */
      sc = sl_bt_advertiser_stop (ble_data_ptr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\r\n",
                    (unsigned int) sc);
        }

      /*
       * Request a change in the connection parameters of a Bluetooth connection.
       */
      sc = sl_bt_connection_set_parameters (ble_data_ptr->connection_handle,
                                            connection_interval_min,
                                            connection_interval_max,
                                            peripheral_latency,
                                            supervision_timeout, 0, 4);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_connection_set_parameters() returned != 0 status=0x%04x",
              (unsigned int) sc);
        }
      displayPrintf (DISPLAY_ROW_CONNECTION, "Connected");

      break;
      //This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      ble_data_ptr->ok_to_send_htm_indications = false;
      ble_data_ptr->connection_open = false;
      ble_data_ptr->bonding_flag = false;
      /* Start advertising of a given advertising set with specified discoverable and connectable modes. */
      sc = sl_bt_advertiser_start (ble_data_ptr->advertisingSetHandle,
                                   sl_bt_advertiser_general_discoverable,
                                   sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x",
                    (unsigned int) sc);
        }
      displayPrintf (DISPLAY_ROW_CONNECTION, "Advertising");
      displayPrintf (DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf (DISPLAY_ROW_9, "");
      displayPrintf (DISPLAY_ROW_ACTION, "");
      displayPrintf (DISPLAY_ROW_PASSKEY, "");
      gpioLed0SetOff ();
      gpioLed1SetOff ();
      sc = sl_bt_sm_delete_bondings ();
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_sm_delete_bondings() returned != 0 status=0x%04x\n\r",
              (unsigned int)sc);
        }
      break;
      //Informational. Triggered whenever the connection parameters are changed and at any time a connection is established
    case sl_bt_evt_connection_parameters_id:
#if LOG_CONNECTION_PARAMETERS
      log_interval = (evt->data.evt_connection_parameters.interval)*1.25;
      log_latency = evt->data.evt_connection_parameters.latency;
      log_timeout = (evt->data.evt_connection_parameters.timeout)*10;
      LOG_INFO("Logged sl_bt_evt_connection_parameters_id values from sl_bt_connection_set_parameters() call.\
          Interval: %dms Latency: %d and Timeout: %dms\n\r",\
          log_interval, log_latency,log_timeout);
#endif
      break;


      //This event indicates that sl_bt_external_signal(myEvent) was called and returns the myEvent value in the event data structure: evt->data.evt_system_external_signal.extsignals
    /*Credit: sl_bt_evt_system_external_signal_id code leveraged from Aditi Vijay Nanaware's A8 submission*/
    case sl_bt_evt_system_external_signal_id:

      // DOS This code looks excessively complex, difficult to determine what is supposed to happen.
      // Perhaps some design first might help, something like this (and yes, it can be as easy as creating
      // comments to use as an outline.

      // ------------------------------------------
      // Deal with GATT DB and button indications
      // ------------------------------------------
      // if evtPB0_pressed event AND ble_data_ptr.button_pressed
      //
      //    update GATT DB
      //
      //    if conditions are correct to send an indication
      //       call sl_bt_gatt_server_send_indication()
      //
      //    if conditions are correct to send an indication, but there is an indication inflight already,
      //       then queue the indication for sending later.
      //
      // end if - evtPB0_pressed event AND ble_data_ptr.button_pressed


      // ---------------------
      // Deal with Security
      // ---------------------
      // if evtPB0_pressed event AND ble_data_ptr.button_pressed, AND
      //    we have a passkey displayed to the user, AND
      //    we are not bonded yet, then:
      //      call sl_bt_sm_passkey_confirm()
      //      clear the "we have a passkey" variable
      // end if


      if (evt->data.evt_system_external_signal.extsignals == evtPB0_pressed)
        {
          button_state[0] = 0;
          button_state[1] = 1;
          sc = sl_bt_gatt_server_write_attribute_value (
              gattdb_button_state, 0, 1, (const uint8_t*) &button_state[1]);
          if (sc != SL_STATUS_OK)
            {
              LOG_ERROR(
                  "sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x",
                  (unsigned int)sc);
            }
          if (ble_data_ptr->connection_open == true)
            {
              displayPrintf (DISPLAY_ROW_9, "Button Pressed");
            }
          if ((ble_data_ptr->bonding_flag == true)
              && (ble_data_ptr->passkey == true))
            {
              sl_bt_sm_passkey_confirm (ble_data_ptr->connection_handle, 1);
            }
          if ((ble_data_ptr->ok_to_send_PB0_indications == true)
              && (ble_data_ptr->connection_open == true))
            {
              if (ble_data_ptr->indication_inflight == false)
                {
                  sc = sl_bt_gatt_server_send_indication (
                      ble_data_ptr->connection_handle, gattdb_button_state,
                      sizeof(button_state), &button_state[0]);
                  if (sc != SL_STATUS_OK)
                    {
                      LOG_ERROR(
                          "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x",
                          (unsigned int)sc);
                    }
                  else
                    {
                      ble_data_ptr->indication_inflight = true;
                    }
                }
              else
                {
                  if (write_queue (gattdb_button_state, 2,
                                   &button_state[0]) == WRITE_FAILURE)
                    LOG_ERROR("write_queue() returned != 0 status=0x%04x",
                              (unsigned int) sc);
                }
            }
        }
      else if (evt->data.evt_system_external_signal.extsignals
          == evtPB0_released)
        {
          button_state[0] = 0;
          button_state[1] = 0;
          sc = sl_bt_gatt_server_write_attribute_value (
              gattdb_button_state, 0, 1, (const uint8_t*) &button_state[1]);
          if (sc != SL_STATUS_OK)
            {
              LOG_ERROR(
                  "sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x",
                  (unsigned int)sc);
            }
          if (ble_data_ptr->connection_open == true)
            {
              displayPrintf (DISPLAY_ROW_9, "Button Released");
              displayPrintf (DISPLAY_ROW_PASSKEY, "");
              displayPrintf (DISPLAY_ROW_ACTION, "");
            }

          if ((ble_data_ptr->ok_to_send_PB0_indications)
              && (ble_data_ptr->connection_open))
            {
              if (ble_data_ptr->indication_inflight == false)
                {
                  sc = sl_bt_gatt_server_send_indication (
                      ble_data_ptr->connection_handle, gattdb_button_state,
                      sizeof(button_state), &button_state[0]);
                  if (sc != SL_STATUS_OK)
                    {
                      LOG_ERROR(
                          "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x",
                          (unsigned int) sc);
                    }
                  else
                    {
                      ble_data_ptr->indication_inflight = true;
                    }
                }
              else
                {
                  if (write_queue (gattdb_button_state, 2,
                                   &button_state[0]) == WRITE_FAILURE)
                    {
                      LOG_ERROR("write_queue() returned != 0 status=0x%04x",
                                (unsigned int) sc);
                    }
                }
            }
        }

      break;
    case sl_bt_evt_system_soft_timer_id:
      //This event indicates that soft timer has expired.
      displayUpdate ();
      /*if block added with the help of Isha Burange*/
      if (ble_data_ptr->indication_inflight == true && (get_queue_depth () > 0))
        {
          size_t buffer_length = 2;
          //read_queue (ble_data_ptr->characteristic_handle, buffer_length, &button_state[2]);

          if (READ_FAILURE
              == read_queue (ble_data_ptr->characteristic_handle, &buffer_length,
                             &button_state[2]))
            {
              LOG_ERROR("read function error\r\n");
            }
          ble_data_ptr->indication_inflight = false;
        }
      break;
      /********************************/
      /*Events only for Slaves/Servers*/
      /********************************/
      /*
       * Indicates either:
       * A local Client Characteristic Configuration descriptor (CCCD) was changed by the remote GATT client, or
       * That a confirmation from the remote GATT Client was received upon a successful reception of the indication I.e. we sent an indication from our server to the client with sl_bt_gatt_server_send_indication()
       */

    case sl_bt_evt_gatt_server_characteristic_status_id:
      /*********
       * HTM
       *********/

      /*******************************************************************************
       * @brief Data structure of the characteristic_status event
       ******************************************************************************/
      //      PACKSTRUCT( struct sl_bt_evt_gatt_server_characteristic_status_s
      //      {
      //        uint8_t  connection;          /**< Connection handle */
      //        uint16_t characteristic;      /**< GATT characteristic handle. This value is
      //                                           normally received from the
      //                                           gatt_characteristic event. */
      //        uint8_t  status_flags;        /**< Enum @ref
      //                                           sl_bt_gatt_server_characteristic_status_flag_t.
      //                                           Describes whether Client Characteristic
      //                                           Configuration was changed or if a
      //                                           confirmation was received. Values:
      //                                             - <b>sl_bt_gatt_server_client_config
      //                                               (0x1):</b> Characteristic client
      //                                               configuration has been changed.
      //                                             - <b>sl_bt_gatt_server_confirmation
      //                                               (0x2):</b> Characteristic confirmation
      //                                               has been received. */
      //        uint16_t client_config_flags; /**< Enum @ref
      //                                           sl_bt_gatt_server_client_configuration_t.
      //                                           This field carries the new value of the
      //                                           Client Characteristic Configuration. If the
      //                                           status_flags is 0x2 (confirmation
      //                                           received), the value of this field can be
      //                                           ignored. */
      //        uint16_t client_config;       /**< The handle of client-config descriptor. */
      //      });
      //sl_bt_api.h: line 5300: sl_bt_gatt_server_characteristic_status_flag_t sl_bt_gatt_server_client_config = 0x1
      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_temperature_measurement
          && evt->data.evt_gatt_server_characteristic_status.status_flags
              == sl_bt_gatt_server_client_config)
        {
          //sl_bt_api.h line 3735: sl_bt_gatt_client_config_flag_t sl_bt_gatt_disable = 0x0
          if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
              == sl_bt_gatt_disable)
            {
              ble_data_ptr->ok_to_send_htm_indications = false;
              displayPrintf (DISPLAY_ROW_TEMPVALUE, "");
              displayPrintf (DISPLAY_ROW_9, "");
              gpioLed0SetOff();
            }
          //sl_bt_api.h line 3735: sl_bt_gatt_client_config_flag_t sl_bt_gatt_indication = 0x02
          else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
              == sl_bt_gatt_indication)
            {
              ble_data_ptr->ok_to_send_htm_indications = true;
              displayPrintf (DISPLAY_ROW_9, "");
              gpioLed0SetOn();

            }
        }
      //sl_bt_api.h line 5302 sl_bt_gatt_server_characteristic_status_flag_t sl_bt_gatt_server_confirmation  = 0x2
      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_temperature_measurement
          && evt->data.evt_gatt_server_characteristic_status.status_flags
              == sl_bt_gatt_server_confirmation)
        {
          ble_data_ptr->indication_inflight = false; //indication reached
        }

      /*********
       * Button PB0
       *********/
      if (evt->data.evt_gatt_server_characteristic_status.characteristic
          == gattdb_button_state)
        {
          if (evt->data.evt_gatt_server_characteristic_status.status_flags
              == sl_bt_gatt_server_client_config)
            {
              if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                  == sl_bt_gatt_indication)
                {
                  ble_data_ptr->ok_to_send_PB0_indications = true;
                  displayPrintf (DISPLAY_ROW_9, "Button Released");
                  gpioLed1SetOn ();
                }
              if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
                  == sl_bt_gatt_disable)
                {
                  ble_data_ptr->ok_to_send_PB0_indications = false;
                  displayPrintf (DISPLAY_ROW_9, "");
                  gpioLed1SetOff ();
                }
            }
          if (evt->data.evt_gatt_server_characteristic_status.status_flags
              == sl_bt_gatt_server_confirmation) //indication reached
            {
              ble_data_ptr->indication_inflight = false;
            }
        }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      LOG_ERROR("Indication timed out\n\r");
      ble_data_ptr->indication_inflight = false; //indication reached
      break;

    case sl_bt_evt_sm_confirm_bonding_id:
      ble_data_ptr->connection_handle = evt->data.evt_sm_confirm_bonding.connection;
      /*
       *
       * Accept or reject the bonding request.
       * @param[in] connection Connection handle
       * @param[in] confirm Acceptance. Values:
       *     - <b>0:</b> Reject
       *     - <b>1:</b> Accept bonding request
       */
      sc = sl_bt_sm_bonding_confirm (ble_data_ptr->connection_handle, 1); //accept
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_sm_bonding_confirm() returned != 0 status=0x%04x",
                    (unsigned int) sc);
        }
      ble_data_ptr->bonding_flag = true;

      break;

    case sl_bt_evt_sm_confirm_passkey_id:
      ble_data_ptr->passkey = evt->data.evt_sm_confirm_passkey.passkey;


      displayPrintf (DISPLAY_ROW_PASSKEY, "Passkey:%d", ble_data_ptr->passkey);
      displayPrintf (DISPLAY_ROW_ACTION, "Confirm with PB0");

      break;

    case sl_bt_evt_sm_bonded_id:

      displayPrintf (DISPLAY_ROW_CONNECTION, "Bonded");
      displayPrintf (DISPLAY_ROW_PASSKEY, " ");
      displayPrintf (DISPLAY_ROW_ACTION, " ");
      ble_data_ptr->bonding_flag = true;

      break;
    case sl_bt_evt_sm_bonding_failed_id:
      LOG_ERROR("Bonding failed with reason: 0x%04x",
                evt->data.evt_sm_bonding_failed.reason);
      break;
    } // end - switch for SERVER

#else

  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();
  switch (SL_BT_MSG_ID(evt->header))
  {
    // ******************************************************
    // Events common to both Servers and Clients
    // ******************************************************
    // --------------------------------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack API commands before receiving this boot event!
    // Including starting BT stack soft timers!
    // --------------------------------------------------------
    case sl_bt_evt_system_boot_id:
      ble_data_ptr->gatt_procedure_complete = false;
      ble_data_ptr->connection_open = false;
      /* Read the Bluetooth identity address used by the device, which can be a public or random static device address. */
      sc = sl_bt_system_get_identity_address(&ble_data_ptr->myAddress, &ble_data_ptr->myAddressType);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      /*
       *  Set the scan mode on the specified PHYs. If the device is currently scanning
       *  for advertising devices on PHYs, new parameters will take effect when
       *  scanning is restarted
       *  @param[in] scan_mode @parblock
       *   Scan mode. Values:
       *     - <b>0:</b> Passive scanning
       *     - <b>1:</b> Active scanning
       */
      sc = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy,0x00); //second argument: 0 Passive scanning
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_scanner_set_mode() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      /*
       * Set the scanning timing parameters on the specified PHYs. If the device is
       * currently scanning for advertising devices on PHYs, new parameters will take
       * effect when scanning is restarted.
       */
      sc = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, scan_interval ,scan_window);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_scanner_set_timing() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      /*
       *  Set the default Bluetooth connection parameters. The values are valid for all
       *  subsequent connections initiated by this device.
       */
      sc = sl_bt_connection_set_default_parameters(connection_interval_min, connection_interval_max,  peripheral_latency,
                                                   supervision_timeout, 0,4);
      /*
       * Start the GAP discovery procedure to scan for advertising devices on the
       * specified scanning PHYs
       */
      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      displayPrintf(DISPLAY_ROW_NAME, "Client");
      displayPrintf(DISPLAY_ROW_BTADDR, "%02x:%02x:%02x:%02x:%02x:%02x",
                    ble_data_ptr->myAddress.addr[0], ble_data_ptr->myAddress.addr[1],
                    ble_data_ptr->myAddress.addr[2], ble_data_ptr->myAddress.addr[3],
                    ble_data_ptr->myAddress.addr[4], ble_data_ptr->myAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT,"A7");
      break;
      //This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      ble_data_ptr->gatt_procedure_complete = false;
      ble_data_ptr->connection_open = true;
      ble_data_ptr->connection_handle = evt->data.evt_connection_opened.connection; //saving connection handle

      displayPrintf(DISPLAY_ROW_BTADDR2, "%02x:%02x:%02x:%02x:%02x:%02x",
                    serverAddress.addr[0],serverAddress.addr[1],
                    serverAddress.addr[2],serverAddress.addr[3],
                    serverAddress.addr[4],serverAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

      break;
      //This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:

      ble_data_ptr->gatt_procedure_complete = false;
      ble_data_ptr->connection_open = false;
      ble_data_ptr->ok_to_send_htm_indications = false;

      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf(DISPLAY_ROW_BTADDR2,"");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT,"A7");
      /*Start the GAP discovery procedure to scan for advertising devices on the
       * specified scanning PHYs
       */
      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x", (unsigned int) sc);
        }

      break;
      //This event indicates that a soft timer has expired.
    case sl_bt_evt_system_soft_timer_id:
      displayUpdate();
      break;

      /*********************************/
      /*Events only for Masters/Clients*/
      /*********************************/
      /*
       * Received for advertising or scan response packets generated by: sl_bt_scanner_start().
       */
    case sl_bt_evt_scanner_scan_report_id:
      /*
       * PACKSTRUCT( struct sl_bt_evt_scanner_scan_report_s
       * {
       * uint8_t    packet_type;       < <b>Bits 0..2</b> : advertising packet type
                                       - <b>000</b> : Connectable scannable
                                         undirected advertising
                                       - <b>001</b> : Connectable undirected
                                         advertising
                                       - <b>010</b> : Scannable undirected
                                         advertising
                                       - <b>011</b> : Non-connectable
                                         non-scannable undirected advertising
                                       - <b>100</b> : Scan Response. Note that
                                         this is received only if the device is
                                         in active scan mode.

                                     <b>Bits 3..4</b> : Reserved for future

                                     <b>Bits 5..6</b> : data completeness
                                       - <b>00:</b> Complete
                                       - <b>01:</b> Incomplete, more data to
                                         come in new events
                                       - <b>10:</b> Incomplete, data truncated,
                                         no more to come

                                     <b>Bit 7</b> : legacy or extended
                                     advertising
                                       - <b>0:</b> Legacy advertising PDUs used
                                       - <b>1:</b> Extended advertising PDUs
                                         used
      bd_addr    address;           < Bluetooth address of the remote device
      uint8_t    address_type;      < Advertiser address type. Values:
                                       - <b>0:</b> Public address
                                       - <b>1:</b> Random address
                                       - <b>255:</b> No address provided
                                         (anonymous advertising)
      .....}*/
      if(evt->data.evt_scanner_scan_report.address_type == 0x00 && evt->data.evt_scanner_scan_report.packet_type == 0x00)       //Address type = Public address, Packet type = Connectable scannable undirected advertising
        {
          if((evt->data.evt_scanner_scan_report.address.addr[0] == serverAddress.addr[0]) &&
              (evt->data.evt_scanner_scan_report.address.addr[1] == serverAddress.addr[1]) &&
              (evt->data.evt_scanner_scan_report.address.addr[2] == serverAddress.addr[2]) &&
              (evt->data.evt_scanner_scan_report.address.addr[3] == serverAddress.addr[3]) &&
              (evt->data.evt_scanner_scan_report.address.addr[4] == serverAddress.addr[4]) &&
              (evt->data.evt_scanner_scan_report.address.addr[5] == serverAddress.addr[5]))
            {
              // Stop scanning for advertising devices
              sc = sl_bt_scanner_stop();
              if (sc != SL_STATUS_OK)
                {
                  LOG_ERROR("sl_bt_scanner_stop() returned != 0 status=0x%04x", (unsigned int) sc);
                }
              /*
               * Connect to an advertising device with the specified initiating PHY on which
               * connectable advertisements on primary advertising channels are received. The
               * Bluetooth stack will enter a state where it continuously scans for the
               * connectable advertising packets from the remote device, which matches the
               * Bluetooth address given as a parameter
               */
              sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                         evt->data.evt_scanner_scan_report.address_type,
                                         sl_bt_gap_1m_phy,
                                         &ble_data_ptr->connection_handle);
              if (sc != SL_STATUS_OK)
                {
                  LOG_ERROR("sl_bt_connection_open() returned != 0 status=0x%04x", (unsigned int) sc);
                }
            }
        }
      break;
      /*We get this event when it’s ok to call the next GATT command*/
    case sl_bt_evt_gatt_procedure_completed_id:
      ble_data_ptr->gatt_procedure_complete = true;
      break;
      /*A GATT service in the remote GATT database was discovered*/
    case sl_bt_evt_gatt_service_id:
      ble_data_ptr->service_handle  = evt->data.evt_gatt_service.service;
      break;
      /*A GATT characteristic in the remote GATT database was discovered*/
    case sl_bt_evt_gatt_characteristic_id:
      ble_data_ptr->characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
      break;
      /*
       * If an indication or notification has been enabled for a characteristic, this event is
       * triggered whenever an indication or notification is received from the remote GATT server.
       */
    case sl_bt_evt_gatt_characteristic_value_id:
      //Credit: sl_bt_evt_gatt_characteristic_value_id event handler code was received from Aditi Vijay Nanaware
      ble_data_ptr->temp_char_value = FLOAT_TO_INT32(evt->data.evt_gatt_characteristic_value.value.data);
      sc = sl_bt_gatt_send_characteristic_confirmation(ble_data_ptr->connection_handle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_gatt_send_characteristic_confirmation() returned != 0 status=0x%04x", (unsigned int) sc);
        }
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d", ble_data_ptr->temp_char_value);
      break;


  }//end switch for CLIENT
#endif

} // handle_ble_event()

#if DEVICE_IS_BLE_SERVER
/**
 * @brief This function reads temperature data from the SI7021 sensor, converts it to IEEE-11073 format,
 *        and writes it to the specified GATT characteristic in the BLE GATT server
 *
 * @param none
 *
 * @returns none
 *
 * @reference ECEN5823 Lecture 10 slides
 */
void
ble_write_temp_from_si7021 (void)
{
  uint8_t *p = htm_temperature_buffer; // HG - Fixed bug with inconsistent address pointing
  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr ();
  UINT8_TO_BITSTREAM(p, flags);
  temperature_in_c = read_temp_from_si7021 ();
  htm_temperature_flt = UINT32_TO_FLOAT(temperature_in_c * 1000, -3);
  UINT32_TO_BITSTREAM(p, htm_temperature_flt);
  sl_status_t sc;
  // -------------------------------
  // Write our local GATT DB
  // -------------------------------
  sc = sl_bt_gatt_server_write_attribute_value (
  gattdb_temperature_measurement, // handle from gatt_db.h
      0, // offset
      4, // length
      (uint8_t*) &(temperature_in_c) // pointer to buffer where data is
      );

  if (sc != SL_STATUS_OK)
    {
      LOG_ERROR(
          "sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x",
          (unsigned int) sc);
    }

  if ((ble_data_ptr->ok_to_send_htm_indications == true)
      && (ble_data_ptr->connection_open == true))
    {
      sc = sl_bt_gatt_server_send_indication (ble_data_ptr->connection_handle,
      gattdb_temperature_measurement, //handle from gatt_db.h
                                              5, &htm_temperature_buffer[0] //in IEEE-11073 format
                                              );
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR(
              "sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x",
              (unsigned int) sc);
        }
      else
        {
          ble_data_ptr->indication_inflight = true;
          displayPrintf (DISPLAY_ROW_TEMPVALUE, "Temp=%d", temperature_in_c);
          //LOG_INFO("Temp in c: %d\n\r", temperature_in_c);
        }
    }
} //ble_write_temp_from_si7021()

#else
/**
 * Convert a Little Endian formatted floating-point value to a 32-bit signed integer.
 * @param value_start_little_endian - Pointer to the Little Endian formatted data.
 * @return A 32-bit signed integer representing the converted value.
 * @reference Assignment 7 document
 */
int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
  uint8_t mantissaSignByte = 0; // these bits will become the upper 8-bits of the mantissa
  int32_t mantissa; // this holds the 24-bit mantissa value with the upper 8-bits as sign bits
  // input data format is:
  // [0] = flags byte
  // [3][2][1] = mantissa (2's complement)
  // [4] = exponent (2's complement)
  // BT value_start_little_endian[0] has the flags byte
  int8_t exponent = (int8_t)value_start_little_endian[4]; // the exponent is a signed 2’s comp value
  // sign extend the mantissa value if the mantissa is negative
  if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
      mantissaSignByte = 0xFF;
  }
  // assemble the mantissa
  mantissa = (int32_t) (value_start_little_endian[1] << 0) |
      (value_start_little_endian[2] << 8) |
      (value_start_little_endian[3] << 16) |
      (mantissaSignByte << 24) ;
  // value = 10^exponent * mantissa; pow() returns a double type
  return (int32_t) (pow( (double) 10, (double) exponent) * (double) mantissa);
} // FLOAT_TO_INT32()
#endif
