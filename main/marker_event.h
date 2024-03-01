#ifndef MARKER_EVENT_H
#define MARKER_EVENT_H

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(MARKER_EVENT);

enum {
    MARKER_EVENT_SENSE1_ON,
    MARKER_EVENT_SENSE1_OFF,
    MARKER_EVENT_SENSE2_ON,
    MARKER_EVENT_SENSE2_OFF,
    MARKER_EVENT_RUNNING_LIGHTS_ON,
    MARKER_EVENT_RUNNING_LIGHTS_OFF,
    MARKER_EVENT_TURN_SIGNAL_ON,
    MARKER_EVENT_TURN_SIGNAL_OFF,
};

#endif  // MARKER_EVENT_H