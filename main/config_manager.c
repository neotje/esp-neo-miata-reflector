#include "config_manager.h"

static const char* TAG = "fs";

esp_event_loop_handle_t config_manager_event_loop;

esp_err_t config_manager_init() {
    ESP_LOGI(TAG, "Initializing nvs flash");

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erasing NVS flash");
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "Failed to erase NVS flash");
        err = nvs_flash_init();
    }

    ESP_RETURN_ON_ERROR(err, TAG, "Failed to initialize NVS flash");

    esp_event_loop_args_t event_loop_args = {
        .queue_size = 10,
        .task_name = "config_manager_event_loop",
        .task_priority = uxTaskPriorityGet(NULL) + CONFIG_CONFIG_MANAGER_TASK_PRIORITY,
        .task_stack_size = 2048,
        .task_core_id = 0,
    };

    err = esp_event_loop_create(&event_loop_args, &config_manager_event_loop);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to create config manager event loop");

    ESP_LOGI(TAG, "Config manager initialized");

    return ESP_OK;
}