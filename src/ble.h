/*
 * ble.h
 *
 *  Created on: 04-Oct-2023
 *      Author: Viswesh
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "app.h"

#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))
// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
typedef struct {
  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType; //0 or 1 for sl_bt_system_get_identity_address();
  // values unique for server
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle; //handle
  uint8_t connection_handle;
  bool connection_open;
  bool ok_to_send_htm_indications;;
  bool indication_inflight;
}ble_data_struct_t;

ble_data_struct_t *get_ble_data_ptr(void);
void handle_ble_event(sl_bt_msg_t *evt);

void ble_write_temp_from_si7021(void);
#endif /* SRC_BLE_H_ */
