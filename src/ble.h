/*
 * File name: ble.h
 * File description: This file declares the BLE APIs
 * Date: 10-Oct-2023
 * Author: Visweshwaran Baskaran viswesh.baskaran@colorado.edu
 * Reference:
 *  [1] ECEN5823 IOT Embedded Firmware lecture slides week 5
 *  [2] Silicon Labs Developer Documentation https://docs.silabs.com/gecko-platform/4.3/platform-emlib-efr32xg1/
 */
#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "app.h"
#include <math.h> // need function prototype for pow()

//Helper macros
#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
//UINT32_TO_FLOAT() takes 2 integer values and converts them into an IEEE-11073 32-bit floating point value.
#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define QUEUE_DEPTH      (16)
#define READ_SUCCESS (bool)false
#define WRITE_SUCCESS (bool)false
#define READ_FAILURE (bool)true
#define WRITE_FAILURE (bool)true

typedef struct
{
  uint16_t charHandle; // Char handle from gatt_db.h
  size_t bufferLength; // Length of buffer in bytes to send
  uint8_t buffer[5]; // The actual data buffer for the indication.
  // Need space for HTM (5 bytes) and button_state (2 bytes)
  // indications, array [0] holds the flags byte.
}queue_element_t;

typedef struct
{
  queue_element_t element[QUEUE_DEPTH];
  uint32_t rptr;
  uint32_t wptr;
  bool empty;
  bool full;
}queue_struct_t ;
// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition

typedef struct
{
  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType; //0 or 1 for sl_bt_system_get_identity_address();
  // values unique for server
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle; //handle
  uint8_t connection_handle;
  bool connection_open;
  bool ok_to_send_htm_indications;
  bool indication_inflight;
  //PB0
  bool ok_to_send_PB0_indications;
  bool bonding_flag;
  bool button_pressed;
  uint32_t passkey;
  // values unique for client
  uint32_t temp_char_value; //for storing characteristic value return
  uint32_t service_handle;
  uint16_t characteristic_handle;
  bool gatt_procedure_complete;
}ble_data_struct_t;

//Function macros

/**
 * @brief Get a pointer to the BLE data structure.
 *
 * @param none
 *
 * @return A pointer to the BLE data structure.
 */
ble_data_struct_t *get_ble_data_ptr(void);

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
void handle_ble_event(sl_bt_msg_t *evt);

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
void ble_write_temp_from_si7021(void);

void ble_send_button_state();
/**
 * Convert a Little Endian formatted floating-point value to a 32-bit signed integer.
 * @param value_start_little_endian - Pointer to the Little Endian formatted data.
 * @return A 32-bit signed integer representing the converted value.
 * @reference Assignment 7 document
 */
int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian);
bool     write_queue (uint16_t charHandle, size_t bufferLength,uint8_t *buffer);
bool     read_queue (uint16_t *charHandle, size_t *bufferLength,uint8_t *buffer);
void     get_queue_status (uint32_t *_wptr, uint32_t *_rptr, bool *_full, bool *_empty);
uint32_t get_queue_depth (void);
#endif /* SRC_BLE_H_ */
