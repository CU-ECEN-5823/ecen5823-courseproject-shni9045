/*
 * ble.h
 *
 * Attributes - soc_thermometer.c
 *              Prof.David Sluiter Lecture 10 & 11
 *  Author @shrikant nimhan shni9045@colorado.edu
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sl_bt_api.h>
#include "em_system.h"
#include "app.h"
#include "em_gpio.h"
#include "efr32bg13p632f512gm48.h"
#include "autogen/gatt_db.h"

#define UINT8_TO_BITSTREAM(p, n)     { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n)    { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                     *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e)        (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))



// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
typedef struct {

  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType;
  bool connection_status;

  // values unique for server
  uint8_t bleconnection;
  uint8_t bond_connection;
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle;

  bool bond_status;

  bool pushbutton_indication_status;
  bool pushbutton_inflight_status;

  bool temp_measure_indication_status;
  bool temp_type_indication_status;
  bool temp_interval_indication_status;

  bool ambient_indication_status;

  bool temp_measure_inflight_status;
  bool temp_type_inflight_status;
  bool temp_interval_inflight_status;

  bool ambient_inflight_status;

  bool enable_measurement;

  // values unique for client
  bd_addr serverAddress;
  uint8_t new_connection;
  bool newconnection_status;

  uint32_t temperature_service;
  uint32_t temperature_characteristic;

  uint32_t button_service;
  uint32_t button_characteristic;

  uint32_t ambient_service;
  uint32_t ambient_characteristic;

  bool indications_enabled;

  uint8_t procedure_completion;


} ble_data_struct_t;


void transmit_ambientdata();

void transmit_tempdata();

void transmit_dutycycle();

void transmit_temptype(sl_bt_msg_t *evt,uint16_t attribute);

void transmit_tempinterval(sl_bt_msg_t *evt,uint16_t attribute);

ble_data_struct_t* getBleDataPtr(void);

void handle_ble_event(sl_bt_msg_t *evt);


#endif /* SRC_BLE_H_ */
