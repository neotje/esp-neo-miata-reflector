#include <stdio.h>

#include "esp_log.h"

#include "fs.h"
#include "marker_event_loop.h"
#include "mode_stack_manager.h"
#include "event_sources/sense_source.h"

static const char* TAG = "marker_app_main";

// marker_event_handler function
static void marker_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "Marker event received: %i", (int)event_id);
}

void app_main(void)
{
    ESP_ERROR_CHECK(fs_init());

    ESP_ERROR_CHECK(marker_event_loop_init());

    ESP_ERROR_CHECK(sense_source_init());

    enmr_mode_stack_manager_config_t mode_stack_manager_config = {
        .idle_mode = g_marker_modes[0],
    };

    ESP_ERROR_CHECK(enmr_mode_stack_manager_init(&mode_stack_manager_config));


    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    

    ESP_ERROR_CHECK(marker_event_loop_on(MARKER_EVENT_SENSE1_ON, marker_event_handler, NULL));
    ESP_ERROR_CHECK(marker_event_loop_on(MARKER_EVENT_SENSE1_OFF, marker_event_handler, NULL));

    ESP_ERROR_CHECK(marker_event_loop_on(MARKER_EVENT_SENSE2_ON, marker_event_handler, NULL));
    ESP_ERROR_CHECK(marker_event_loop_on(MARKER_EVENT_SENSE2_OFF, marker_event_handler, NULL));
}
