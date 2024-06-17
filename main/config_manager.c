#include "config_manager.h"

ESP_EVENT_DEFINE_BASE(CONFIG_MANAGER_EVENT);

/*
PRIVATE
*/
static const char* TAG = "config_manager";

esp_event_loop_handle_t config_manager_event_loop;

esp_err_t config_trigger_update_event(const char* namespace, const char* key) {
    config_manager_event_update_t event_data = {
        .namespace = namespace,
        .key = key
    };

    return esp_event_post_to(
        config_manager_event_loop,
        CONFIG_MANAGER_EVENT,
        CONFIG_MANAGER_EVENT_UPDATE,
        &event_data,
        sizeof(config_manager_event_update_t),
        portMAX_DELAY
    );
}


/*
PUBLIC
*/

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
        .queue_size = CONFIG_CONFIG_MANAGER_EVENT_QUEUE_SIZE,
        .task_name = "config_manager_event_loop",
        .task_priority = uxTaskPriorityGet(NULL) + CONFIG_CONFIG_MANAGER_EVENT_LOOP_PRIORITY,
        .task_stack_size = CONFIG_CONFIG_MANAGER_EVENT_LOOP_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };

    err = esp_event_loop_create(&event_loop_args, &config_manager_event_loop);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to create config manager event loop");

    ESP_LOGI(TAG, "Config manager initialized");

    return ESP_OK;
}

esp_err_t config_manager_set_u8(const char *namespace, const char *key, uint8_t value)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_set_u8(handle, key, value);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set value");

    err = nvs_commit(handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to commit value");

    nvs_close(handle);

    err = config_trigger_update_event(namespace, key);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to trigger update event");

    return ESP_OK;
}

esp_err_t config_manager_get_u8(const char *namespace, const char *key, uint8_t *out)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_get_u8(handle, key, out);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get value");

    nvs_close(handle);

    return ESP_OK;
}

esp_err_t config_manager_set_u16(const char *namespace, const char *key, uint16_t value)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_set_u16(handle, key, value);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set value");

    err = nvs_commit(handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to commit value");

    nvs_close(handle);

    err = config_trigger_update_event(namespace, key);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to trigger update event");

    return ESP_OK;
}

esp_err_t config_manager_get_u16(const char *namespace, const char *key, uint16_t *out)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_get_u16(handle, key, out);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get value");

    nvs_close(handle);

    return ESP_OK;
}

esp_err_t config_manager_set_i32(const char *namespace, const char *key, int32_t value)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_set_i32(handle, key, value);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set value");

    err = nvs_commit(handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to commit value");

    nvs_close(handle);

    err = config_trigger_update_event(namespace, key);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to trigger update event");

    return ESP_OK;
}

esp_err_t config_manager_get_i32(const char *namespace, const char *key, int32_t *out)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_get_i32(handle, key, out);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get value");

    nvs_close(handle);

    return ESP_OK;
}

esp_err_t config_manager_set_blob(const char *namespace, const char *key, const void *value, size_t size)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_set_blob(handle, key, value, size);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set value");

    err = nvs_commit(handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to commit value");

    nvs_close(handle);

    err = config_trigger_update_event(namespace, key);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to trigger update event");

    return ESP_OK;
}

esp_err_t config_manager_get_blob(const char *namespace, const char *key, void *out, size_t *size)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to open namespace");

    err = nvs_get_blob(handle, key, out, size);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get value");

    nvs_close(handle);

    return ESP_OK;
}

esp_err_t config_manager_register_update_handler(esp_event_handler_t event_handler, esp_event_handler_instance_t *instance)
{
    return esp_event_handler_instance_register_with(
        config_manager_event_loop, 
        CONFIG_MANAGER_EVENT, 
        CONFIG_MANAGER_EVENT_UPDATE, 
        event_handler,
        NULL,
        instance
    );
}

esp_err_t config_manager_unregister_update_handler(esp_event_handler_instance_t instance)
{
    return esp_event_handler_instance_unregister_with(
        config_manager_event_loop,
        CONFIG_MANAGER_EVENT,
        CONFIG_MANAGER_EVENT_UPDATE,
        instance
    );
}
