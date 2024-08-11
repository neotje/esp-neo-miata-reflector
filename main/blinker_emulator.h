/**
 * @file blinker_emulator.h
 * @version 1.0
 */

#ifndef BLINKER_EMULATOR_H
#define BLINKER_EMULATOR_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_timer.h"
#include "argtable3/argtable3.h"

#include "mode_event_linker.h"
#include "sense_wire_event_source.h"

esp_err_t blinker_emulator_init();

#endif