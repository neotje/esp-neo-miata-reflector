/**
 * @file mode_event_linker.h
 * @version 1.1
 */

#ifndef MODE_EVENT_LINKER_H
#define MODE_EVENT_LINKER_H

#include <stdint.h>
#include "esp_check.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_console.h"
#include "esp_timer.h"
#include "argtable3/argtable3.h"
#include "mode_event.h"
#include "config_manager.h"
#include "mode_stack_manager.h"

typedef enum {
	ENTER_ACTION,
	EXIT_ACTION
} mode_event_link_action_t;

typedef struct {
	uint8_t mode_id;
	int32_t event_id;
	mode_event_link_action_t action;
} __attribute__((packed)) mode_event_link_t;

/**
 * @brief Initialize the mode event linker: event loop, load links from Config Manager(NVS) and set up the event handler.
 * 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_init();

/**
 * @brief Add a link between a mode and an event. The link will be saved to Config Manager(NVS). Each mode cannot have duplicate events for enter or exit.
 * 
 * @param mode_id 
 * @param event_id 
 * @param action 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_add(uint8_t mode_id, int32_t event_id, mode_event_link_action_t action);

/**
 * @brief Remove a link between a mode and an event. The link will be removed from Config Manager(NVS).
 * 
 * @param mode_id 
 * @param event_id 
 * @param action 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_remove(uint8_t mode_id, int32_t event_id, mode_event_link_action_t action);

/**
 * @brief Post an event to the event loop. The event will be processed by the mode event linker.
 * 
 * @param event_id 
 * @param event_data 
 * @param event_data_size 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_post(int32_t event_id, const void* event_data, size_t event_data_size);

/**
 * @brief Print all the links between modes and events.
 * 
 */
void mode_event_linker_print();

/**
 * @brief Register an event handler for a specific event.
 * 
 * @param event_id 
 * @param event_handler 
 * @param evnent_handler_arg 
 * @param instance 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_register_handler(int32_t event_id, esp_event_handler_t event_handler, void* evnent_handler_arg, esp_event_handler_instance_t* instance);

/**
 * @brief Unregister an event handler for a specific event.
 * 
 * @param event_id 
 * @param instance 
 * @return esp_err_t 
 */
esp_err_t mode_event_linker_unregister_handler(int32_t event_id, esp_event_handler_instance_t instance);

#endif