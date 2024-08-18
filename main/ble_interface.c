/**
 * @file ble_interface.c
 * @version 1.0
 */

#include "ble_interface.h"

/*
PRIVATE
*/
static const char *TAG = "BLE_INTERFACE";

/*
PUBLIC
*/
esp_err_t ble_interface_init()
{
    ESP_RETURN_ON_ERROR(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT), TAG, "release classic bt memory failed");

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_bt_controller_init(&bt_cfg), TAG, "initialize controller failed");

    ESP_RETURN_ON_ERROR(esp_bt_controller_enable(ESP_BT_MODE_BLE), TAG, "enable controller failed");

    ESP_RETURN_ON_ERROR(esp_bluedroid_init(), TAG, "initialize bluedroid failed");

    ESP_RETURN_ON_ERROR(esp_bluedroid_enable(), TAG, "enable bluedroid failed");

    return ESP_OK;
}
