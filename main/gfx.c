/**
 * @file gfx.c
 * @version 1.0
 */

#include "gfx.h"

/*
PRIVATE
*/
static const char* TAG = "gfx";
static const char *NAMESPACE = "gfx";
static const char *TRANSITION_DURATION_KEY = "transition";

uint32_t* output_buffer = NULL;
size_t output_buffer_size = 0;

uint32_t* previous_buffer = NULL;
size_t previous_buffer_size = 0;

uint32_t* draw_buffer = NULL;
size_t draw_buffer_size = 0;

int64_t transitionStartTime = -1;

led_strip_handle_t led_strip_handle = NULL;

static void gfx_draw_loop() {}

/*
PUBLIC
*/
esp_err_t gfx_init()
{
    ESP_LOGI(TAG, "Initializing");

    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_BOARD_LED_STRIP_PIN,
        .max_leds = CONFIG_LED_STRIP_NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle), TAG, "Failed to create LED strip handle");

    // test led strip
    ESP_RETURN_ON_ERROR(led_strip_set_pixel(led_strip_handle, 0, 255, 0, 0), TAG, "Failed to set pixel");
    ESP_RETURN_ON_ERROR(led_strip_refresh(led_strip_handle), TAG, "Failed to refresh LED strip");
    ESP_RETURN_ON_ERROR(led_strip_clear(led_strip_handle), TAG, "Failed to clear LED strip");

    // allocate memory for buffers
    output_buffer_size = CONFIG_LED_STRIP_NUM_LEDS;
    output_buffer = (uint32_t*)malloc(output_buffer_size * sizeof(uint32_t));
    if (output_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
        return ESP_ERR_NO_MEM;
    }

    previous_buffer_size = CONFIG_LED_STRIP_NUM_LEDS;
    previous_buffer = (uint32_t*)malloc(previous_buffer_size * sizeof(uint32_t));
    if (previous_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for previous buffer");
        return ESP_ERR_NO_MEM;
    }

    draw_buffer_size = CONFIG_LED_STRIP_NUM_LEDS;
    draw_buffer = (uint32_t*)malloc(draw_buffer_size * sizeof(uint32_t));
    if (draw_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for draw buffer");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}