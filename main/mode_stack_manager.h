#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_check.h"
#include "esp_log.h"

#include "marker_mode.h"

#ifndef MODESTACKMANAGER_H
#define MODESTACKMANAGER_H

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
