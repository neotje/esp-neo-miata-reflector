/**
 * @file config_manager.h
 * @version 1.1
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include "esp_check.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "config_event.h"

typedef struct {
    const char *namespace;
    const char *key;
} config_manager_event_update_t;

/**
 * @brief initialize flash nvs, erase if necessary and create event loop
 * 
 * @return esp_err_t 
 */
esp_err_t config_manager_init();

/**
 * @brief set uint8_t value in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value value to store
 * @return esp_err_t 
 */
esp_err_t config_manager_set_u8(const char *namespace, const char *key, uint8_t value);

/**
 * @brief get uint8_t value from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store value
 * @return esp_err_t 
 */
esp_err_t config_manager_get_u8(const char *namespace, const char *key, uint8_t *out);

/**
 * @brief set uint16_t value in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value value to store
 * @return esp_err_t 
 */
esp_err_t config_manager_set_u16(const char *namespace, const char *key, uint16_t value);

/**
 * @brief get uint16_t value from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store value
 * @return esp_err_t 
 */
esp_err_t config_manager_get_u16(const char *namespace, const char *key, uint16_t *out);

/**
 * @brief set int32_t value in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value value to store
 * @return esp_err_t 
 */
esp_err_t config_manager_set_i32(const char *namespace, const char *key, int32_t value);

/**
 * @brief get int32_t value from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store value
 * @return esp_err_t 
 */
esp_err_t config_manager_get_i32(const char *namespace, const char *key, int32_t *out);

/**
 * @brief set uint32_t value in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value value to store
 * @return esp_err_t 
 */
esp_err_t config_manager_set_u32(const char *namespace, const char *key, uint32_t value);

/**
 * @brief get uint32_t value from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store value
 * @return esp_err_t 
 */
esp_err_t config_manager_get_u32(const char *namespace, const char *key, uint32_t *out);

/**
 * @brief set int64_t value in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value value to store
 * @return esp_err_t 
 */
esp_err_t config_manager_set_i64(const char *namespace, const char *key, int64_t value);

/**
 * @brief get int64_t value from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store value
 * @return esp_err_t 
 */
esp_err_t config_manager_get_i64(const char *namespace, const char *key, int64_t *out);

/**
 * @brief set variable length binary data in namespace
 * 
 * @param namespace namespace to store value in
 * @param key key to store value under
 * @param value pointer to binary data
 * @param size size of binary data
 * @return esp_err_t 
 */
esp_err_t config_manager_set_blob(const char *namespace, const char *key, const void *value, size_t size);

/**
 * @brief get variable length binary data from namespace
 * 
 * @param namespace namespace to get value from
 * @param key key to get value from
 * @param out pointer to store binary data
 * @param size size of binary data
 * @return esp_err_t 
 */
esp_err_t config_manager_get_blob(const char *namespace, const char *key, void *out, size_t *size);

/**
 * @brief add event handler to listen for updates
 * 
 * @param event_handler
 * @param instance 
 * @return esp_err_t 
 */
esp_err_t config_manager_register_update_handler(esp_event_handler_t event_handler, esp_event_handler_instance_t *instance);

/**
 * @brief remove event handler from listening for updates
 * 
 * @param instance 
 * @return esp_err_t 
 */
esp_err_t config_manager_unregister_update_handler(esp_event_handler_instance_t instance);

#endif  // CONFIG_MANAGER_H