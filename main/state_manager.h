#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>
#include <string.h>
#include "esp_check.h"
#include "esp_system.h"
#include "esp_event.h"

#include "state_manager_event.h"

typedef struct {
    const char *key;
    void *value;
    size_t size;
} state_entry_t;

esp_err_t state_manager_init();

esp_err_t state_manager_set(const char *key, void *value, size_t size);

esp_err_t state_manager_get(const char *key, void **out, size_t* out_size);

esp_err_t state_manager_register_update_handler(esp_event_handler_t event_handler, esp_event_handler_instance_t *instance);

esp_err_t state_manager_unregister_update_handler(esp_event_handler_instance_t instance);

void state_manager_print_entries();

#endif