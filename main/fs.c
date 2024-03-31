#include "fs.h"

static const char* TAG = "fs";

esp_err_t fs_init() {
    ESP_LOGI(TAG, "Initializing file system");
    
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erasing NVS flash");
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "Failed to erase NVS flash");
        err = nvs_flash_init();
    }

    ESP_RETURN_ON_ERROR(err, TAG, "Failed to initialize NVS flash");

    return ESP_OK;
}