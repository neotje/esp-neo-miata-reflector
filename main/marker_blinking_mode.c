#include "marker_blinking_mode.h"

/*
PRIVATE
*/
static const char* TAG = "marker_blinking_mode";

static void marker_blinking_mode_enter()
{
    ESP_LOGI(TAG, "Entering marker_blinking_mode");
}

static void marker_blinking_mode_function()
{
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

    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_SENSE1_ON, ENTER_ACTION);
    mode_event_linker_add(CONFIG_BLINKING_MODE_ID, MARKER_EVENT_SENSE1_OFF, EXIT_ACTION);

    return ESP_OK;
}
