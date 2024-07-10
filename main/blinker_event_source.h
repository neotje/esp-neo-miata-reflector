/**
 * @file blinker_event_source.h
 * @version 1.0
 */

#ifndef BLINKER_EVENT_SOURCE_H
#define BLINKER_EVENT_SOURCE_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_timer.h"

#include "mode_event_linker.h"
#include "sense_wire_event_source.h"
#include "config_manager.h"
#include "state_manager.h"

/**
 * @brief Initialize the blinker event source: Configure the blinker, subscribe to the config manager update event, and load the configuration.
 * 
 * @return esp_err_t 
 */
esp_err_t blinker_event_source_init();

/**
 * @brief Set the sense wire index for the blinker and (de)init event handlers.
 * 
 * @param index The index of the sense wire.
 * @return esp_err_t 
 */
esp_err_t blinker_event_source_set_sense_wire(int index);

#endif // BLINKER_EVENT_SOURCE_H