#ifndef MARKERMODE_H
#define MARKERMODE_H

#include <stdint.h>
#include "esp_err.h"

typedef void (* mode_function_t)( /* void * */ );

typedef struct {
    uint8_t brightness;
    uint8_t priority;

    int32_t *enter_event_ids;
    int32_t enter_event_ids_count;

    int32_t *exit_event_ids;
    int32_t exit_event_ids_count;
} enmr_marker_mode_config_t;

typedef struct {

} enmr_marker_mode_state_t;

typedef struct {
    const char*                 name;
    mode_function_t             mode_function;

    enmr_marker_mode_config_t   config;
    enmr_marker_mode_state_t    state;
} enmr_marker_mode_t;

typedef enum {
    MARKER_MODE_EVENT_ENTER,
    MARKER_MODE_EVENT_EXIT,
} enmr_marker_mode_event_t;


static const uint8_t IDLE_MODE_PRIORITY = 0;

extern enmr_marker_mode_t *g_marker_modes[];
extern const size_t MARKER_MODES_COUNT;


esp_err_t marker_mode_add_event(int32_t event_id, enmr_marker_mode_t* mode, enmr_marker_mode_event_t event_t);

esp_err_t marker_mode_remove_event(int32_t event_id, enmr_marker_mode_t* mode, enmr_marker_mode_event_t event_t);

#endif // MARKERMODE_H
