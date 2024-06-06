#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include "esp_check.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"

ESP_EVENT_DECLARE_BASE(CONFIG_MANAGER_EVENT);

enum {
    CONFIG_MANAGER_EVENT_UPDATE,
};

/**
 * @brief initialize flash nvs, erase if necessary and create event loop
 * 
 * @return esp_err_t 
 */
esp_err_t config_manager_init();

esp_err_t config_manager_set_u8(const char *namespace, const char *key, uint8_t value);
esp_err_t config_manager_get_u8(const char *namespace, const char *key, uint8_t *out);

esp_err_t config_manager_set_u16(const char *namespace, const char *key, uint16_t value);
esp_err_t config_manager_get_u16(const char *namespace, const char *key, uint16_t *out);

esp_err_t config_manager_set_blob(const char *namespace, const char *key, const void *value, size_t size);
esp_err_t config_manager_get_blob(const char *namespace, const char *key, void *out, size_t size);

esp_err_t config_manager_register_update_handler(esp_event_handler_t event_handler, esp_event_handler_instance_t *instance);
esp_err_t config_manager_unregister_update_handler(esp_event_handler_instance_t instance);

#endif  // CONFIG_MANAGER_H