#include "mode_stack_manager.h"

/*
PRIVATE
*/
static const char *TAG = "ModeStackManager";

stack_manager_mode_t** available_modes = NULL;
size_t available_modes_count = 0;

stack_manager_mode_t** stack = NULL;
size_t stack_size = 0;

TaskHandle_t mode_stack_manager_task_handle = NULL;

void mode_stack_manager_task(void* args)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t is_mode_on_stack(uint8_t id)
{
    for (size_t i = 0; i < stack_size; i++)
    {
        if (stack[i]->id == id)
        {
            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t mode_stack_manager_sort()
{
    return ESP_OK;
}

/*
PUBLIC
*/
esp_err_t mode_stack_manager_init()
{
    xTaskCreate(
        mode_stack_manager_task,
        "mode_stack_manager_task",
        CONFIG_MODE_STACK_MANAGER_TASK_STACK_SIZE,
        NULL,
        uxTaskPriorityGet(NULL) + CONFIG_MODE_STACK_MANAGER_TASK_PRIORITY,
        &mode_stack_manager_task_handle
    );

    configASSERT(mode_stack_manager_task_handle);

    return ESP_OK;
}

esp_err_t mode_stack_manager_add_mode(stack_manager_mode_t* mode)
{
    stack_manager_mode_t* temp;
    ESP_RETURN_ON_ERROR(mode_stack_manager_get_mode(mode->id, &temp), TAG, "Mode already exists");

    stack_manager_mode_t** temp_modes = available_modes;

    if (temp_modes == NULL)
    {
        temp_modes = (stack_manager_mode_t**)malloc(sizeof(stack_manager_mode_t*));
    }
    else
    {
        temp_modes = (stack_manager_mode_t**)realloc(temp_modes, (available_modes_count + 1) * sizeof(stack_manager_mode_t*));
    }

    ESP_RETURN_ON_FALSE(temp_modes != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for available modes");

    temp_modes[available_modes_count] = mode;
    available_modes_count++;
    available_modes = temp_modes;

    return ESP_OK;
}

esp_err_t mode_stack_manager_enter_mode(uint8_t id)
{
    stack_manager_mode_t* mode;
    ESP_RETURN_ON_ERROR(mode_stack_manager_get_mode(id, &mode), TAG, "Mode not found");

    if (is_mode_on_stack(id) == ESP_OK)
    {
        return ESP_ERR_INVALID_STATE;
    }

    stack_manager_mode_t** temp_stack = stack;

    if (temp_stack == NULL)
    {
        temp_stack = (stack_manager_mode_t**)malloc(sizeof(stack_manager_mode_t*));
    }
    else
    {
        temp_stack = (stack_manager_mode_t**)realloc(temp_stack, (stack_size + 1) * sizeof(stack_manager_mode_t*));
    }

    ESP_RETURN_ON_FALSE(temp_stack != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for mode stack");

    temp_stack[stack_size] = mode;
    stack_size++;

    ESP_RETURN_ON_ERROR(mode_stack_manager_sort(), TAG, "Failed to sort mode stack");

    mode->mode_enter_func();

    return ESP_OK;
}

esp_err_t mode_stack_manager_exit_mode(uint8_t id)
{
    stack_manager_mode_t* mode;
    ESP_RETURN_ON_ERROR(mode_stack_manager_get_mode(id, &mode), TAG, "Mode not found");

    if (is_mode_on_stack(id) != ESP_OK)
    {
        return ESP_ERR_INVALID_STATE;
    }

    for (size_t i = 0; i < stack_size; i++)
    {
        if (stack[i]->id == id)
        {
            stack[i]->mode_exit_func();

            // remove mode from stack
            

            ESP_RETURN_ON_ERROR(mode_stack_manager_sort(), TAG, "Failed to sort mode stack");

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;

}

esp_err_t mode_stack_manager_get_mode(uint8_t id, stack_manager_mode_t** mode)
{
    for (size_t i = 0; i < available_modes_count; i++)
    {
        if (available_modes[i]->id == id)
        {
            *mode = available_modes[i];
            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}
