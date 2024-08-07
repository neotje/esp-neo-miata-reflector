/**
 * @file gfx.h
 * @version 1.1
 */

#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include "esp_check.h"
#include "esp_system.h"
#include "led_strip.h"
#include "config_manager.h"
#include "state_manager.h"

esp_err_t gfx_init();

esp_err_t gfx_set(size_t index, uint32_t color);

esp_err_t gfx_set_rgb(size_t index, uint8_t r, uint8_t g, uint8_t b);

esp_err_t gfx_clear();

esp_err_t gfx_start_transition();

#endif