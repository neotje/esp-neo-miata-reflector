#include <stdio.h>

#include "esp_log.h"

#include "mode_stack_manager.h"

void app_main(void)
{
    enmr_marker_mode_t idle_mode = MARKER_MODE_INIT_IDLE_MODE();

    enmr_marker_mode_config_t daytime_mode_config = {
        .brightness = 100,
        .priority = 1,
    };
    enmr_marker_mode_t daytime_mode = {
        .name = "Daytime Mode",
        .config = &daytime_mode_config,
        .state = NULL,
    };

    enmr_marker_mode_config_t blinker_mode_config = {
        .brightness = 100,
        .priority = 2,
    };
    enmr_marker_mode_t blinker_mode = {
        .name = "Blinker Mode",
        .config = &blinker_mode_config,
        .state = NULL,
    };

    enmr_mode_stack_manager_config_t mode_stack_manager_config = {
        .idle_mode = &idle_mode,
    };

    ESP_ERROR_CHECK(enmr_mode_stack_manager_init(&mode_stack_manager_config));

    enmr_mode_stack_manager_print();

    ESP_ERROR_CHECK(enmr_mode_stack_manager_enter(&daytime_mode));
    enmr_mode_stack_manager_print();

    ESP_ERROR_CHECK(enmr_mode_stack_manager_enter(&blinker_mode));
    enmr_mode_stack_manager_print();

    ESP_ERROR_CHECK(enmr_mode_stack_manager_exit(&daytime_mode));
    enmr_mode_stack_manager_print();

    ESP_ERROR_CHECK(enmr_mode_stack_manager_enter(&daytime_mode));
    enmr_mode_stack_manager_print();
}
