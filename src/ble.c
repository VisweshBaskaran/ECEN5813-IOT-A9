/*
 * File name: ble.h
 * File description: This file declares the BLE APIs
 * Date: 04-Oct-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *  [1] ECEN5813 IOT Embedded Firmware lecture slides week 5
 *  [2] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */


#include "src/ble.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//Macros
#define LOG_CONNECTION_PARAMETERS 1

//Variables required
ble_data_struct_t ble_data_ptr;
uint32_t advertising_interval_max = 0x190, advertising_interval_min = 0x190; //Set the Advertising minimum and maximum to 250mS. 250/0.625 = 400 = 0x190
uint16_t connection_interval_max = 0x3c, connection_interval_min = 0x3c; //Set Connection Interval minimum and maximum to 75mS. 75/1.25 =60 = 0x3c
uint16_t peripheral_latency = 4;
uint16_t supervision_timeout = 0x4c; //(peripheral_latency+1)*(connection_interval * 2) = 750ms/10 = 0x4b;
uint16_t log_timeout, log_latency, log_interval;

//htm temperature variables
uint8_t htm_temperature_buffer[5];
uint8_t *p = htm_temperature_buffer;
uint32_t htm_temperature_flt;
uint8_t flags = 0x00;
int32_t temperature_in_c;

/**
 * @brief Get a pointer to the BLE data structure.
 *
 * @param none
 *
 * @return A pointer to the BLE data structure.
 */
ble_data_struct_t *get_ble_data_ptr(void)
{
  return &ble_data_ptr;
}

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
void handle_ble_event(sl_bt_msg_t *evt)
{
  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();
  sl_status_t sc = SL_STATUS_OK;
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

      /*
       * Read the Bluetooth identity address used by the device, which can be a public
       * or random static device address.
       */
      sc = sl_bt_system_get_identity_address(&ble_data_ptr->myAddress, &ble_data_ptr->myAddressType);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }

      /*
       * Create an advertising set. The handle of the created advertising set is
       * returned in response.
       */
      sc = sl_bt_advertiser_create_set(&ble_data_ptr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }
      /*
       * Set the advertising timing parameters of the given advertising set. This
       * setting will take effect next time that advertising is enabled.
       */
      sc = sl_bt_advertiser_set_timing(ble_data_ptr->advertisingSetHandle,advertising_interval_min, advertising_interval_max ,0,0);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }
      /*Start advertising of a given advertising set with specified discoverable and connectable modes*/
      //@reference Immediate line of code based on soc_thermometer example app.c line 157-160
      sc = sl_bt_advertiser_start(ble_data_ptr->advertisingSetHandle,sl_bt_advertiser_general_discoverable,sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }
      break;
      //This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      /* @reference
            Immediate line of code generated using ChatGPT with the following prompt:
            "After event: sl_bt_evt_connection_opened_id
             sl_bt_advertiser_stop() is called,
             Step to do: Save this connection handle. Use this value in all subsequent calls that require a connect handle, when this connection is open to the client"
       */
      ble_data_ptr->connection_handle = evt->data.evt_connection_opened.connection;  // Save the connection handle for future use
      ble_data_ptr->connection_open = true;
      /*
       * Stop the advertising of the given advertising set. Counterpart with @ref
       * sl_bt_advertiser_start.
       */
      sc = sl_bt_advertiser_stop(ble_data_ptr->advertisingSetHandle);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\r\n", (unsigned int) sc);
        }

      /*
       * Request a change in the connection parameters of a Bluetooth connection.
       */
      sc = sl_bt_connection_set_parameters(ble_data_ptr->connection_handle,
                                           connection_interval_min,
                                           connection_interval_max,
                                           peripheral_latency,
                                           supervision_timeout, 0,0);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_connection_set_parameters() returned != 0 status=0x%04x", (unsigned int) sc);
        }

      break;
      //This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      ble_data_ptr->connection_open = false;
      /* Start advertising of a given advertising set with specified discoverable and connectable modes. */
      sc = sl_bt_advertiser_start(ble_data_ptr->advertisingSetHandle,sl_bt_advertiser_general_discoverable,sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int) sc);
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
    case sl_bt_evt_system_external_signal_id:
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

      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement && evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config)
        {
          //sl_bt_api.h line 3735: sl_bt_gatt_client_config_flag_t sl_bt_gatt_disable = 0x0
          if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_disable)
            {
              ble_data_ptr->ok_to_send_htm_indications = false;
            }
          //sl_bt_api.h line 3735: sl_bt_gatt_client_config_flag_t sl_bt_gatt_indication = 0x02
          else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_indication)
            {
              ble_data_ptr->ok_to_send_htm_indications = true;
            }
        }
      //sl_bt_api.h line 5302 sl_bt_gatt_server_characteristic_status_flag_t sl_bt_gatt_server_confirmation  = 0x2
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement && evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation)
        {
          ble_data_ptr->indication_inflight = false; //indication reached
        }
      break;

    case sl_bt_evt_gatt_server_indication_timeout_id:
      LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x", (unsigned int) sc);
      ble_data_ptr->indication_inflight = false; //indication reached
      break;
  } // end - switch
} // handle_ble_event()

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
void ble_write_temp_from_si7021(void)
{
  ble_data_struct_t *ble_data_ptr = get_ble_data_ptr();
  UINT8_TO_BITSTREAM(p, flags);
  temperature_in_c = read_temp_from_si7021();
  htm_temperature_flt = UINT32_TO_FLOAT(temperature_in_c*1000, -3);
  UINT32_TO_BITSTREAM(p, htm_temperature_flt);
  sl_status_t sc;
  // -------------------------------
  // Write our local GATT DB
  // -------------------------------
  sc = sl_bt_gatt_server_write_attribute_value(
      gattdb_temperature_measurement, // handle from gatt_db.h
      0, // offset
      4, // length
      (uint8_t *)&(temperature_in_c) // pointer to buffer where data is
  );

  if (sc != SL_STATUS_OK)
    {
      LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x", (unsigned int) sc);
    }

  if((ble_data_ptr->ok_to_send_htm_indications == true)  && (ble_data_ptr->connection_open == true))
    {
      sc = sl_bt_gatt_server_send_indication(
          ble_data_ptr->connection_handle,
          gattdb_temperature_measurement, //handle from gatt_db.h
          5,
          &htm_temperature_buffer[0] //in IEEE-11073 format
      );
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x", (unsigned int) sc);
      }
      else
        {
          ble_data_ptr->indication_inflight = true;
          //LOG_INFO("Temp=%d\n\r", temperature_in_c);
        }
    }
} //ble_write_temp_from_si7021()

