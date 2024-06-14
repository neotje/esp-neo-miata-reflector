#ifndef MODESTACKMANAGER_H
#define MODESTACKMANAGER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_check.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "state_manager.h"

typedef void (* mode_function_t)( /* void * */ );

typedef struct {
    uint8_t id; // unique id
	const char* name;

    uint8_t priority;
	
	mode_function_t mode_enter_func;
	mode_function_t mode_function;
	mode_function_t mode_exit_func;
} stack_manager_mode_t;

esp_err_t mode_stack_manager_init();

esp_err_t mode_stack_manager_add_mode(stack_manager_mode_t* mode);

esp_err_t mode_stack_manager_enter_mode(uint8_t id);

esp_err_t mode_stack_manager_exit_mode(uint8_t id);

esp_err_t mode_stack_manager_get_current_mode();

esp_err_t mode_stack_manager_get_mode(uint8_t id, stack_manager_mode_t** mode);

#endif //MODESTACKMANAGER_H
