#include "mode_stack_manager.h"

/*
PRIVATE
*/
static const char *TAG = "mode_stack_manager";

stack_manager_mode_t** available_modes = NULL;
size_t available_modes_count = 0;

stack_manager_mode_t** stack = NULL;
size_t stack_size = 0;

TaskHandle_t mode_stack_manager_task_handle = NULL;

void mode_stack_manager_task(void* args)
{
    stack_manager_mode_t* previous_mode = NULL;
    stack_manager_mode_t* current_mode = NULL;

    while(1)
    {
        if (mode_stack_manager_get_current_mode(&current_mode) == ESP_OK)
        {
            if (previous_mode != current_mode)
            {
                ESP_LOGI(TAG, "Mode %s is running", current_mode->name);
            }

            if (current_mode->mode_function != NULL) current_mode->mode_function();

            previous_mode = current_mode;
        }

        vTaskDelay(pdMS_TO_TICKS(17));
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
    stack_manager_mode_t* temp;
    for (size_t i = 0; i < stack_size; i++)
    {
        for (size_t j = i + 1; j < stack_size; j++)
        {
            if (stack[i]->priority < stack[j]->priority)
            {
                temp = stack[i];
                stack[i] = stack[j];
                stack[j] = temp;
            }
        }
    }

    if (mode_stack_manager_get_current_mode(&temp) == ESP_OK)
    {
        uint8_t current_id = temp->id;

        state_manager_set("current_mode", &current_id, sizeof(uint8_t));
    }

    return ESP_OK;
}

/*
PUBLIC
*/
esp_err_t mode_stack_manager_init(stack_manager_mode_t* idle_mode)
{
    ESP_RETURN_ON_FALSE(idle_mode != NULL, ESP_ERR_INVALID_ARG, TAG, "Idle mode is NULL");

    idle_mode->priority = IDLE_MODE_PRIORITY;
    ESP_RETURN_ON_ERROR(mode_stack_manager_add_mode(idle_mode), TAG, "Failed to add idle mode");

    ESP_RETURN_ON_ERROR(mode_stack_manager_enter_mode(idle_mode->id), TAG, "Failed to enter idle mode");

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
    ESP_RETURN_ON_FALSE(mode != NULL, ESP_ERR_INVALID_ARG, TAG, "Mode is NULL");

    stack_manager_mode_t* temp;
    ESP_RETURN_ON_FALSE(mode_stack_manager_get_mode(mode->id, &temp) != ESP_OK, ESP_ERR_INVALID_STATE, TAG, "Mode already exists");

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

    ESP_LOGI(TAG, "Added mode %s with id %d", mode->name, mode->id);

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
    stack = temp_stack; // you dumb fuck forget to store the temporary stack to the global stack.

    ESP_RETURN_ON_ERROR(mode_stack_manager_sort(), TAG, "Failed to sort mode stack"); // Illigal address accessed :(

    if (mode->mode_enter_func != NULL) mode->mode_enter_func();

    return ESP_OK;
}

esp_err_t mode_stack_manager_exit_mode(uint8_t id)
{
    ESP_RETURN_ON_FALSE(id != IDLE_MODE_PRIORITY, ESP_ERR_INVALID_ARG, TAG, "Cannot exit idle mode");

    stack_manager_mode_t* mode;
    ESP_RETURN_ON_ERROR(mode_stack_manager_get_mode(id, &mode), TAG, "Mode not found");

    if (is_mode_on_stack(id) != ESP_OK)
    {
        return ESP_OK;
    }

    for (size_t i = 0; i < stack_size; i++)
    {
        if (stack[i]->id == id)
        {
            if(stack[i]->mode_exit_func != NULL) stack[i]->mode_exit_func();

            // remove mode from stack
            for (size_t j = i; j < stack_size - 1; j++)
            {
                stack[j] = stack[j + 1];
            }

            stack_manager_mode_t** temp = (stack_manager_mode_t**)realloc(stack, (stack_size - 1) * sizeof(stack_manager_mode_t*));

            ESP_RETURN_ON_FALSE(temp != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for mode stack");

            stack_size--;
            stack = temp;

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;

}

esp_err_t mode_stack_manager_get_current_mode(stack_manager_mode_t **mode)
{
    if (stack_size == 0)
    {
        return ESP_ERR_NOT_FOUND;
    }

    *mode = stack[0];

    return ESP_OK;
}

esp_err_t mode_stack_manager_get_mode(uint8_t id, stack_manager_mode_t** mode)
{
    for (size_t i = 0; i < available_modes_count; i++)
    {
        if (available_modes[i]->id == id)
        {
            if (mode != NULL) *mode = available_modes[i];
            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

void mode_stack_manager_print_stack()
{
    ESP_LOGI(TAG, "Mode stack:");
    for (size_t i = 0; i < stack_size; i++)
    {
        ESP_LOGI(TAG, "Mode %d: %s", stack[i]->id, stack[i]->name);
    }
}
