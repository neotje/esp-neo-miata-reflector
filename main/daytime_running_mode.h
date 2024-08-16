/**
 * @file daytime_running_mode.h
 * @version 1.0
 */

#ifndef DAYTIME_RUNNING_MODE_H
#define DAYTIME_RUNNING_MODE_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

#include "mode_stack_manager.h"
#include "mode_event_linker.h"
#include "blinker_event_source.h"
#include "gfx.h"

esp_err_t daytime_running_mode_init();

#endif // DAYTIME_RUNNING_MODE_H