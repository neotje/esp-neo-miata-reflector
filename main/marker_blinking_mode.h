/**
 * @file marker_blinking_mode.h
 * @version 1.0
 */

#ifndef MARKER_BLINKING_MODE_H
#define MARKER_BLINKING_MODE_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"

#include "mode_stack_manager.h"
#include "mode_event_linker.h"
#include "blinker_event_source.h"
#include "gfx.h"

typedef enum {
    BLINKING_STYLE_BLINK,
    BLINKING_STYLE_SEQUENTIAL,
} blinking_style_t;

esp_err_t marker_blinking_mode_init();

#endif // MARKER_BLINKING_MODE_H