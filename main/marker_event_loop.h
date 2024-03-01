#ifndef MARKER_EVENT_LOOP_H
#define MARKER_EVENT_LOOP_H

#include "esp_check.h"
#include "esp_log.h"
#include "marker_event.h"

esp_err_t marker_event_loop_init();

esp_event_loop_handle_t marker_event_loop_get();

esp_err_t marker_event_loop_trigger(int32_t event_id, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait);

esp_err_t marker_event_loop_on(int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg);

#endif  // MARKER_EVENT_LOOP_H  