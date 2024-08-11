/**
 * @file gfx.h
 * @version 1.0
 */

#ifndef GFX_H
#define GFX_H

#include <stdint.h>

#include "esp_check.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "led_strip.h"

#include "config_manager.h"
#include "state_manager.h"

/**
 * @brief Initialize the graphics module: create the LED strip handle, allocate memory for the output buffer and test/clear the LED strip.
 * 
 * @return esp_err_t 
 */
esp_err_t gfx_init();

/**
 * @brief Set the color of a specific pixel.
 * 
 * @param index index of the pixel to set
 * @param color color to set
 * @return esp_err_t 
 */
esp_err_t gfx_set(size_t index, uint32_t color);

/**
 * @brief Set the RGB color of a specific pixel.
 * 
 * @param index index of the pixel to set
 * @param r red part of the color
 * @param g green part of the color
 * @param b blue part of the color
 * @return esp_err_t 
 */
esp_err_t gfx_set_rgb(size_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Clear the LED strip.
 * 
 * @return esp_err_t 
 */
esp_err_t gfx_clear();

/**
 * @brief Start the transition of the LED strip. Save the current buffer to the previous buffer and start the transition.
 * 
 * @return esp_err_t 
 */
esp_err_t gfx_start_transition();

/**
 * @brief Linearly interpolate between two colors.
 * 
 * @param from 
 * @param to 
 * @param amount 
 * @return uint32_t 
 */
uint32_t gfx_lerp_color(uint32_t from, uint32_t to, float amount);

size_t gfx_get_length();

#endif