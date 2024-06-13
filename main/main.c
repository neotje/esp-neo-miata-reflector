#include <stdio.h>

#include "esp_log.h"

#include "config_manager.h"
#include "state_manager.h"

static const char* TAG = "marker_app_main";


void app_main(void)
{
    ESP_ERROR_CHECK(config_manager_init());

    ESP_ERROR_CHECK(state_manager_init());


    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
