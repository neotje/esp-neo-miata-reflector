/**
 * @file ble_interface.h
 * @version 1.0
 */

#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "config_manager.h"
#include "state_manager.h"

esp_err_t ble_interface_init();

#endif // BLE_INTERFACE_H