#include <stdint.h>

#ifndef MARKERMODE_H
#define MARKERMODE_H

typedef struct {
    uint8_t brightness;
    uint8_t priority;
} enmr_marker_mode_config_t;

typedef struct {

} enmr_marker_mode_state_t;

typedef struct {
    const char*                 name;

    enmr_marker_mode_config_t*  config;
    enmr_marker_mode_state_t*   state;
} enmr_marker_mode_t;


extern enmr_marker_mode_config_t g_marker_mode_idle_config;

static const uint8_t IDLE_MODE_PRIORITY = 0;

#define MARKER_MODE_INIT_IDLE_MODE() { \
    .name = "Idle Mode", \
    .config = &g_marker_mode_idle_config, \
    .state = NULL, \
}

#endif // CONFIGMANAGER_H
