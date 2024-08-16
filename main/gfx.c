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

static const size_t LED_COUNT = CONFIG_LED_STRIP_NUM_LEDS;

uint32_t* output_buffer = NULL;
uint32_t* previous_buffer = NULL;
uint32_t* draw_buffer = NULL;

int64_t transition_start_time = -1;
uint32_t transition_duration = 0;
#define transition_duration_us (transition_duration * 1000)
#define transition_end_time (transition_start_time + transition_duration_us)

led_strip_handle_t led_strip_handle = NULL;

esp_timer_handle_t draw_loop_timer_handle = NULL;

double gfx_ease_in_out_cubic(double t) {
    return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

void set_is_transitioning(bool value) {
    void* value_ptr = (void*)&value;

    if(state_manager_set("is_transitioning", value_ptr, sizeof(bool)) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set is_transitioning");
    }
}

bool get_is_transitioning() {
    bool is_transitioning = false;
    void* value;

    if(state_manager_get("is_transitioning", &value, NULL) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get is_transitioning");
    } else {
        is_transitioning = *(bool*)value;
    }
    
    return is_transitioning;
}

void update_transition_duration() {
    if(config_manager_get_u32(NAMESPACE, TRANSITION_DURATION_KEY, &transition_duration) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get transition duration");
    }
}

double fps;
int64_t previous_draw_time = 0;

static void gfx_draw_loop(void* args) {
    // calculate fps
    int64_t current_time = esp_timer_get_time();
    fps = 1.0 / ((current_time - previous_draw_time) / 1000000.0);
    previous_draw_time = current_time;

    float progress = 0.0f;
    bool is_transitioning = get_is_transitioning();

    if (is_transitioning && esp_timer_get_time() <= transition_end_time) {
        progress = (float)(esp_timer_get_time() - transition_start_time) / transition_duration_us;
    }

    if (is_transitioning && esp_timer_get_time() > transition_end_time) {
        is_transitioning = false;
    }

    if (is_transitioning) {
        progress = gfx_ease_in_out_cubic(progress);
        for (size_t i = 0; i < LED_COUNT; i++) {
            output_buffer[i] = gfx_lerp_color(previous_buffer[i], draw_buffer[i], progress);
        }
    } else {
        memcpy(output_buffer, draw_buffer, LED_COUNT * sizeof(uint32_t));
    }

    for (size_t i = 0; i < gfx_get_length(); i++) {
        if(led_strip_set_pixel(led_strip_handle, i, (output_buffer[i] >> 16) & 0xFF, (output_buffer[i] >> 8) & 0xFF, output_buffer[i] & 0xFF) != ESP_OK) {
            ESP_LOGW(TAG, "Failed to set pixel");
        }
    }

    if(led_strip_refresh(led_strip_handle) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to refresh LED strip");
    }
}

static void gfx_config_update_handler(void *args, esp_event_base_t base, int32_t id, void *event_data) {
    config_manager_event_update_t *event = (config_manager_event_update_t*)event_data;

    if (strcmp(event->namespace, NAMESPACE) == 0) {
        if (strcmp(event->key, TRANSITION_DURATION_KEY) == 0) {
            update_transition_duration();
        }
    }
}


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

    set_is_transitioning(false);

    ESP_RETURN_ON_ERROR(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle), TAG, "Failed to create LED strip handle");
    
    //ESP_RETURN_ON_ERROR(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip_handle), TAG, "Failed to create LED strip handle");

    // test led strip
    ESP_RETURN_ON_ERROR(led_strip_set_pixel(led_strip_handle, 0, 255, 0, 0), TAG, "Failed to set pixel");
    ESP_RETURN_ON_ERROR(led_strip_refresh(led_strip_handle), TAG, "Failed to refresh LED strip");
    ESP_RETURN_ON_ERROR(led_strip_clear(led_strip_handle), TAG, "Failed to clear LED strip");

    // allocate memory for buffers
    output_buffer = (uint32_t*)malloc(LED_COUNT * sizeof(uint32_t));
    if (output_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
        return ESP_ERR_NO_MEM;
    }

    previous_buffer = (uint32_t*)malloc(LED_COUNT * sizeof(uint32_t));
    if (previous_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for previous buffer");
        return ESP_ERR_NO_MEM;
    }

    draw_buffer = (uint32_t*)malloc(LED_COUNT * sizeof(uint32_t));
    if (draw_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for draw buffer");
        return ESP_ERR_NO_MEM;
    }

    // clear buffers
    memset(output_buffer, 0, LED_COUNT * sizeof(uint32_t));
    memset(previous_buffer, 0, LED_COUNT * sizeof(uint32_t));
    memset(draw_buffer, 0, LED_COUNT * sizeof(uint32_t));

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(gfx_config_update_handler, NULL), TAG, "Failed to register config update handler");

    update_transition_duration();

    esp_timer_create_args_t draw_loop_timer_args = {
        .arg = NULL,
        .callback = &gfx_draw_loop,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "gfx_draw_loop"
    };

    ESP_RETURN_ON_ERROR(esp_timer_create(&draw_loop_timer_args, &draw_loop_timer_handle), TAG, "Failed to create draw loop timer");

    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(draw_loop_timer_handle, 1000000 / CONFIG_LED_STRIP_FPS), TAG, "Failed to start draw loop timer");

    //gfx_set_rgb(0, 255, 0, 255);
    //gfx_start_transition();

    return ESP_OK;
}

esp_err_t gfx_set(size_t index, uint32_t color)
{
    if (index >= gfx_get_length()) {
        ESP_LOGW(TAG, "Index out of bounds");
        return ESP_ERR_INVALID_ARG;
    }

    draw_buffer[index] = color;

    return ESP_OK;
}

esp_err_t gfx_set_rgb(size_t index, uint8_t r, uint8_t g, uint8_t b)
{
    return gfx_set(index, (r << 16) | (g << 8) | b);
}

esp_err_t gfx_clear()
{
    memset(draw_buffer, 0, gfx_get_length() * sizeof(uint32_t));

    return ESP_OK;
}

esp_err_t gfx_start_transition()
{
    memccpy(previous_buffer, output_buffer, LED_COUNT, sizeof(uint32_t));

    transition_start_time = esp_timer_get_time();
    set_is_transitioning(true);

    return ESP_OK;
}

uint32_t gfx_lerp_color(uint32_t from, uint32_t to, double amount)
{
    uint8_t from_r = (from >> 16) & 0xFF;
    uint8_t from_g = (from >> 8) & 0xFF;
    uint8_t from_b = from & 0xFF;

    uint8_t to_r = (to >> 16) & 0xFF;
    uint8_t to_g = (to >> 8) & 0xFF;
    uint8_t to_b = to & 0xFF;

    uint8_t r = round(from_r + (to_r - from_r) * amount);
    uint8_t g = round(from_g + (to_g - from_g) * amount);
    uint8_t b = round(from_b + (to_b - from_b) * amount);

    return (r << 16) | (g << 8) | b;
}

void gfx_draw_line(uint32_t color, size_t start, size_t end)
{
    // clip start and end
    for (size_t i = start; i <= end; i++) {
        gfx_set(i, color);
    }
}

void gfx_draw_linef(uint32_t color, double start, double length)
{
    double brightness_first_pixel = fmin(1.0 - (start - (long)start), length);
    double remaining = fmin(length, gfx_get_length() - start);
    size_t position = (size_t)start;

    if (remaining > 0.0)
    {
        uint32_t color_first_pixel = gfx_rgb_set_brightness(color, brightness_first_pixel);
        gfx_set(position++, color_first_pixel);
        remaining -= brightness_first_pixel;
    }


    while (remaining >= 1.0)
    {
        gfx_set(position++, color);
        remaining--;
    }

    if (remaining > 0.0)
    {
        uint32_t color_last_pixel = gfx_rgb_set_brightness(color, remaining);
        gfx_set(position, color_last_pixel);
    }

}

uint32_t gfx_rgb_set_brightness(uint32_t color, double brightness)
{
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = round(r * brightness);
    g = round(g * brightness);
    b = round(b * brightness);

    return (r << 16) | (g << 8) | b;
}

size_t gfx_get_length() {
    return LED_COUNT;
}