#ifndef MODESTACKMANAGER_H
#define MODESTACKMANAGER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_check.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "marker_mode.h"

/*
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
*/

typedef struct {
    size_t stack_size;
    enmr_marker_mode_t** stack;
} enmr_mode_stack_manager_state_t;

typedef struct {
    enmr_marker_mode_t* idle_mode;
} enmr_mode_stack_manager_config_t;

esp_err_t enmr_mode_stack_manager_init(enmr_mode_stack_manager_config_t* cnf);

esp_err_t enmr_mode_stack_manager_enter(enmr_marker_mode_t* mode);

esp_err_t enmr_mode_stack_manager_exit(enmr_marker_mode_t* mode);

esp_err_t enmr_mode_stack_manager_sort();

enmr_marker_mode_t* enmr_mode_stack_manager_get_current_mode();

void enmr_mode_stack_manager_print();

#endif //MODESTACKMANAGER_H
