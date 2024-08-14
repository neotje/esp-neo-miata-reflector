#include "marker_blinking_mode.h"

/*
PRIVATE
*/
static const char *TAG = "marker_blinking_mode";
static const char *NAMESPACE = "blinker";
static const char *ON_COLOR_KEY = "color";
static const char *STYLE_KEY = "style";

int64_t previous_blink_time = 0;
int64_t previous_loop_time = 0;
double loop_dt = 0;

bool is_blinker_on = false;

uint32_t current_color = 0;
uint32_t target_color = 0;

uint32_t on_color = 0xff6600;
blinking_style_t style = BLINKING_STYLE_BLINK;

esp_event_handler_instance_t blinking_event_handler_instance;

void start_blink()
{
    previous_blink_time = esp_timer_get_time();
    is_blinker_on = true;
    current_color = on_color;
}

void start_sequential()
{
    previous_blink_time = esp_timer_get_time();
    is_blinker_on = true;
    gfx_draw_line(0, 0, gfx_get_length() - 1);
}

void blink_loop(int64_t current_time, int64_t timeout)
{
    if (is_blinker_on)
    {
        target_color = on_color;
    }
    else
    {
        target_color = 0;
    }

    //current_color = gfx_lerp_color(current_color, target_color, loop_dt / (timeout / 1000000.0));
    current_color = target_color;
    gfx_draw_line(current_color, 0, gfx_get_length() - 1);
}

void sequential_loop(int64_t current_time, int64_t timeout) {
    if (is_blinker_on)
    {
        double progress = (current_time - previous_blink_time) / (double)timeout;

        gfx_draw_linef(on_color, 0, progress * gfx_get_length());
    }
    else
    {
        gfx_draw_line(0, 0, gfx_get_length() - 1);
    }    
}

void (*get_loop_function(blinking_style_t style))(int64_t, int64_t)
{
    switch (style)
    {
    case BLINKING_STYLE_BLINK:
        return &blink_loop;
    case BLINKING_STYLE_SEQUENTIAL:
        return &sequential_loop;
    default:
        return &blink_loop;
    }
}

void (*get_start_function(blinking_style_t style))()
{
    switch (style)
    {
    case BLINKING_STYLE_BLINK:
        return &start_blink;
    case BLINKING_STYLE_SEQUENTIAL:
        return &start_sequential;
    default:
        return &start_blink;
    }
}

static void marker_blinking_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGI(TAG, "Received event %li", id);

    if (id == MARKER_EVENT_TURN_SIGNAL_ON)
    {
        start_blink();
    }
}

static void marker_blinking_config_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    config_manager_event_update_t *update = (config_manager_event_update_t *)event_data;

    if (strcmp(update->namespace, NAMESPACE) == 0 && id == CONFIG_MANAGER_EVENT_UPDATE)
    {
        if (strcmp(update->key, ON_COLOR_KEY) == 0)
        {
            config_manager_get_u32(NAMESPACE, ON_COLOR_KEY, &on_color);
        }
        else if (strcmp(update->key, STYLE_KEY) == 0)
        {
            config_manager_get_u32(NAMESPACE, STYLE_KEY, (uint32_t *)&style);
        }
    }
}

static void marker_blinking_mode_enter()
{
    ESP_LOGI(TAG, "Entering marker_blinking_mode");
    gfx_start_transition();
    get_start_function(style)();
}

static void marker_blinking_mode_function()
{
    int64_t current_time = esp_timer_get_time();
    int64_t timeout;
    loop_dt = (current_time - previous_loop_time) / 1000000.0;
    previous_loop_time = current_time;

    if (is_blinker_on)
    {
        timeout = blinker_event_source_get_on_duration();
    }
    else
    {
        timeout = blinker_event_source_get_off_duration();
    }

    get_loop_function(style)(current_time, timeout);

    if (current_time >= previous_blink_time + timeout)
    {
        previous_blink_time = current_time;
        is_blinker_on = !is_blinker_on;
    }
}

static void marker_blinking_mode_exit()
{
    ESP_LOGI(TAG, "Exiting marker_blinking_mode");
}

stack_manager_mode_t marker_blinking_mode = {
    .id = CONFIG_BLINKING_MODE_ID,
    .name = "marker_blinking_mode",
    .priority = CONFIG_BLINKING_MODE_PRIO,
    .mode_enter_func = &marker_blinking_mode_enter,
    .mode_function = &marker_blinking_mode_function,
    .mode_exit_func = &marker_blinking_mode_exit,
};

/*
PUBLIC
*/
esp_err_t marker_blinking_mode_init()
{
    ESP_LOGI(TAG, "Initializing marker_blinking_mode");

    ESP_RETURN_ON_ERROR(mode_stack_manager_add_mode(&marker_blinking_mode), TAG, "Failed to add marker_blinking_mode");

    ESP_RETURN_ON_ERROR(mode_event_linker_register_handler(MARKER_EVENT_TURN_SIGNAL_ON, marker_blinking_event_handler, NULL, &blinking_event_handler_instance), TAG, "Failed to register handler for MARKER_EVENT_TURN_SIGNAL_ON");

    config_manager_get_u32(NAMESPACE, ON_COLOR_KEY, &on_color);
    config_manager_get_u32(NAMESPACE, STYLE_KEY, (uint32_t *)&style);

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(marker_blinking_config_handler, NULL), TAG, "Failed to register update handler");

    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_TURN_SIGNAL_ON, ENTER_ACTION);
    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_TURN_SIGNAL_OFF, EXIT_ACTION);

    return ESP_OK;
}
