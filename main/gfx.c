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

int64_t transition_start_time = -1;
uint32_t transition_duration = 0;
#define transition_duration_us (transition_duration * 1000)
#define transition_end_time (transition_start_time + transition_duration_us)

led_strip_handle_t led_strip_handle = NULL;

esp_timer_handle_t draw_loop_timer_handle = NULL;

float ease_in_out_cubic(float t) {
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

static void gfx_draw_loop(void* args) {
    float progress = 0.0f;
    bool is_transitioning = get_is_transitioning();

    if (is_transitioning && esp_timer_get_time() <= transition_end_time) {
        progress = (float)(esp_timer_get_time() - transition_start_time) / transition_duration_us;
    }

    if (is_transitioning && esp_timer_get_time() > transition_end_time) {
        is_transitioning = false;
    }

    if (is_transitioning) {
        progress = ease_in_out_cubic(progress);
        for (size_t i = 0; i < draw_buffer_size; i++) {
            output_buffer[i] = gfx_lerp_color(previous_buffer[i], draw_buffer[i], progress);
        }
    } else {
        memccpy(output_buffer, draw_buffer, draw_buffer_size, sizeof(uint32_t));
    }

    for (size_t i = 0; i < output_buffer_size; i++) {
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

    // clear buffers
    memset(output_buffer, 0, output_buffer_size * sizeof(uint32_t));
    memset(previous_buffer, 0, previous_buffer_size * sizeof(uint32_t));
    memset(draw_buffer, 0, draw_buffer_size * sizeof(uint32_t));

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
    if (index >= draw_buffer_size) {
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
    memset(draw_buffer, 0, draw_buffer_size * sizeof(uint32_t));

    return ESP_OK;
}

esp_err_t gfx_start_transition()
{
    memccpy(previous_buffer, output_buffer, output_buffer_size, sizeof(uint32_t));

    transition_start_time = esp_timer_get_time();
    set_is_transitioning(true);

    return ESP_OK;
}

uint32_t gfx_lerp_color(uint32_t from, uint32_t to, float amount)
{
    uint8_t from_r = (from >> 16) & 0xFF;
    uint8_t from_g = (from >> 8) & 0xFF;
    uint8_t from_b = from & 0xFF;

    uint8_t to_r = (to >> 16) & 0xFF;
    uint8_t to_g = (to >> 8) & 0xFF;
    uint8_t to_b = to & 0xFF;

    uint8_t r = from_r + (to_r - from_r) * amount;
    uint8_t g = from_g + (to_g - from_g) * amount;
    uint8_t b = from_b + (to_b - from_b) * amount;

    return (r << 16) | (g << 8) | b;
}

size_t gfx_get_length() {
    return output_buffer_size;
}