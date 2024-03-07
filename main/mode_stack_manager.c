#include "mode_stack_manager.h"

/*
 * PRIVATE
 */

static const char *TAG = "ModeStackManager";

enmr_mode_stack_manager_state_t *state = NULL;
enmr_mode_stack_manager_config_t *config = NULL;

TaskHandle_t enmr_mode_stack_manager_task_handle = NULL;

static void enmr_mode_stack_manager_task(void *args)
{
    ESP_LOGI(TAG, "Mode stack manager task started");
    while (1)
    {
        enmr_marker_mode_t *current_mode = enmr_mode_stack_manager_get_current_mode();

        if (current_mode != NULL && current_mode->mode_function != NULL)
        {
            current_mode->mode_function();
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_MODE_LOOP_DELAY));
    }
}



/*
 * PUBLIC
 */

enmr_mode_stack_manager_state_t *enmr_mode_stack_manager_new_state(enmr_marker_mode_t *idle_mode)
{
    enmr_mode_stack_manager_state_t *new_state = (enmr_mode_stack_manager_state_t *)malloc(sizeof(enmr_mode_stack_manager_state_t));

    if (new_state == NULL)
    {
        return NULL;
    }

    new_state->stack_size = 1;
    new_state->stack = (enmr_marker_mode_t **)malloc(sizeof(enmr_marker_mode_t *));

    if (new_state->stack == NULL)
    {
        free(new_state);
        return NULL;
    }

    new_state->stack[0] = idle_mode;

    return new_state;
}

esp_err_t enmr_mode_stack_manager_is_initilized()
{
    return state != NULL ? ESP_OK : ESP_ERR_INVALID_STATE;
}

esp_err_t enmr_mode_stack_manager_init(enmr_mode_stack_manager_config_t *cnf)
{
    if (state != NULL || config != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (
        cnf == NULL ||
        cnf->idle_mode == NULL ||
        cnf->idle_mode->config.priority != IDLE_MODE_PRIORITY)
    {
        return ESP_ERR_INVALID_ARG;
    }

    state = enmr_mode_stack_manager_new_state(cnf->idle_mode);

    if (state == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    config = cnf;

    ESP_LOGI(TAG, "Starting mode stack manager task");
    xTaskCreate(
        enmr_mode_stack_manager_task, 
        "mode_stack_manager_task", 
        2048, 
        NULL, 
        uxTaskPriorityGet(NULL) + CONFIG_MODE_STACK_TASK_PRIORITY, 
        &enmr_mode_stack_manager_task_handle
    );

    return ESP_OK;
}

esp_err_t enmr_mode_stack_manager_enter(enmr_marker_mode_t *mode)
{
    ESP_RETURN_ON_ERROR(enmr_mode_stack_manager_is_initilized(), TAG, "Mode stack manager is not initialized");

    enmr_marker_mode_t **stack = state->stack;

    // Check if mode is already in the stack
    for (int i = 0; i < state->stack_size; i++)
    {
        enmr_marker_mode_t *current_mode = stack[i];

        if (
            current_mode->config.priority == mode->config.priority ||
            stack[i] == mode)
        {
            return ESP_FAIL;
        }
    }

    // Add mode to the stack
    size_t new_stack_size = state->stack_size + 1;
    enmr_marker_mode_t **new_stack = (enmr_marker_mode_t **)realloc(state->stack, new_stack_size * sizeof(enmr_marker_mode_t *));

    if (new_stack == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    state->stack = new_stack;
    state->stack[state->stack_size] = mode;
    state->stack_size = new_stack_size;

    return enmr_mode_stack_manager_sort();
}

esp_err_t enmr_mode_stack_manager_exit(enmr_marker_mode_t *mode)
{
    // Cannot remove the idle mode
    if (mode->config.priority == IDLE_MODE_PRIORITY || mode == config->idle_mode)
    {
        return ESP_ERR_INVALID_ARG;
    }

    enmr_marker_mode_t **stack = state->stack;

    // Remove mode from the stack
    for (size_t i = 0; i < state->stack_size; i++)
    {
        enmr_marker_mode_t *current_mode = stack[i];

        if (current_mode == mode)
        {
            for (size_t j = i; j < state->stack_size - 1; j++)
            {
                stack[j] = stack[j + 1];
            }

            size_t new_stack_size = state->stack_size - 1;
            enmr_marker_mode_t **new_stack = (enmr_marker_mode_t **)realloc(state->stack, new_stack_size * sizeof(enmr_marker_mode_t *));

            if (new_stack == NULL)
            {
                return ESP_ERR_NO_MEM;
            }

            state->stack = new_stack;
            state->stack_size = new_stack_size;

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t enmr_mode_stack_manager_sort()
{
    // Sort modes in the stack by priority (descending)
    enmr_marker_mode_t **stack = state->stack;

    for (size_t i = 0; i < state->stack_size; i++)
    {
        for (size_t j = i + 1; j < state->stack_size; j++)
        {
            if ((stack[i]->config.priority) < (stack[j]->config.priority))
            {
                enmr_marker_mode_t *temp = stack[i];
                stack[i] = stack[j];
                stack[j] = temp;
            }
        }
    }

    return ESP_OK;
}

enmr_marker_mode_t *enmr_mode_stack_manager_get_current_mode()
{
    return state->stack[0];
}

void enmr_mode_stack_manager_print()
{
    ESP_LOGI(TAG, "Mode stack:");

    for (size_t i = 0; i < state->stack_size; i++)
    {
        ESP_LOGI(TAG, "Mode %d: %s", i, state->stack[i]->name);
    }
}