#include "marker_blinking_mode.h"

/*
PRIVATE
*/
static const char* TAG = "marker_blinking_mode";

int64_t previous_blink_time = 0;
bool is_blinker_on = false;
uint32_t current_color = 0;
uint32_t target_color = 0;

esp_event_handler_instance_t blinking_event_handler_instance;

void start_blink()
{
    previous_blink_time = esp_timer_get_time();
    is_blinker_on = true;
    current_color = 0xff6600;
    target_color = current_color;
}

static void marker_blinking_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGI(TAG, "Received event %li", id);

    if (id == MARKER_EVENT_TURN_SIGNAL_ON)
    {
        start_blink();
    }
}

static void marker_blinking_mode_enter()
{
    ESP_LOGI(TAG, "Entering marker_blinking_mode");
    start_blink();
}

static void marker_blinking_mode_function()
{
    int64_t current_time = esp_timer_get_time();
    int64_t timeout;

    if (is_blinker_on)
    {
        timeout = blinker_event_source_get_on_duration();
    }
    else
    {
        timeout = blinker_event_source_get_off_duration();
    }

    if (current_time >= previous_blink_time + timeout)
    {
        previous_blink_time = current_time;
        if (is_blinker_on)
        {
            target_color = 0x000000;
        }
        else
        {
            target_color = 0xff6600;
        }

        is_blinker_on = !is_blinker_on;
    }

    //current_color = gfx_lerp_color(current_color, target_color, 0.1);
    current_color = target_color;

    for (size_t i = 0; i < gfx_get_length(); i++)
    {
        gfx_set(i, current_color);
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

    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_TURN_SIGNAL_ON, ENTER_ACTION);
    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_TURN_SIGNAL_OFF, EXIT_ACTION);

    return ESP_OK;
}
