#include <stdio.h>

#include "esp_log.h"

#include "config_manager.h"
#include "state_manager.h"
#include "mode_stack_manager.h"
#include "mode_event_linker.h"

static const char* TAG = "marker_app_main";

stack_manager_mode_t idle_mode = {
    .id = 0,
    .name = "idle",
    .priority = IDLE_MODE_PRIORITY,
};

void app_main(void)
{
    ESP_ERROR_CHECK(config_manager_init());

    ESP_ERROR_CHECK(state_manager_init());

    ESP_ERROR_CHECK(mode_stack_manager_init(&idle_mode));

    ESP_ERROR_CHECK(mode_event_linker_init());

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
