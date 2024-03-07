#include "marker_mode.h"

enmr_marker_mode_t IDLE_MODE = {
    .name = "Idle Mode",
    .mode_function = NULL,
    .config = {
        .brightness = 0,
        .priority = IDLE_MODE_PRIORITY,
        .enter_event_ids = NULL,
        .enter_event_ids_count = 0,
        .exit_event_ids = NULL,
        .exit_event_ids_count = 0,
    },
    .state = {},
};

enmr_marker_mode_t DAYTIME_MODE = {
    .name = "Daytime Mode",
    .mode_function = NULL,
    .config = {
        .brightness = 100,
        .priority = 1,
        .enter_event_ids = NULL,
        .enter_event_ids_count = 0,
        .exit_event_ids = NULL,
        .exit_event_ids_count = 0,
    },
    .state = {},
};

enmr_marker_mode_t BLINKING_MODE = {
    .name = "Blinking Mode",
    .mode_function = NULL,
    .config = {
        .brightness = 100,
        .priority = 2,
        .enter_event_ids = NULL,
        .enter_event_ids_count = 0,
        .exit_event_ids = NULL,
        .exit_event_ids_count = 0,
    },
    .state = {},
};

enmr_marker_mode_t *g_marker_modes[] = {
    &IDLE_MODE,
    &DAYTIME_MODE,
    &BLINKING_MODE,
};

const size_t MARKER_MODES_COUNT = sizeof(g_marker_modes) / sizeof(enmr_marker_mode_t *);

esp_err_t marker_mode_add_event(int32_t event_id, enmr_marker_mode_t *mode, enmr_marker_mode_event_t event_t)
{
    for (int i = 0; i < mode->config.enter_event_ids_count; i++)
    {
        if (event_id == mode->config.enter_event_ids[i])
        {
            return ESP_ERR_INVALID_STATE;
        }
    }

    for (int i = 0; i < mode->config.exit_event_ids_count; i++)
    {
        if (event_id == mode->config.exit_event_ids[i])
        {
            return ESP_ERR_INVALID_STATE;
        }
    }

    int32_t *target_event_ids;
    int32_t *target_event_ids_count;

    if (event_t == MARKER_MODE_EVENT_ENTER)
    {
        target_event_ids = mode->config.enter_event_ids;
        target_event_ids_count = &mode->config.enter_event_ids_count;
    }
    else if (event_t == MARKER_MODE_EVENT_EXIT)
    {
        target_event_ids = mode->config.exit_event_ids;
        target_event_ids_count = &mode->config.exit_event_ids_count;
    }
    else
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (target_event_ids == NULL)
    {
        target_event_ids = malloc(sizeof(int32_t));

        if (target_event_ids == NULL)
        {
            return ESP_ERR_NO_MEM;
        }
    }

    int32_t *new_event_ids = realloc(target_event_ids, (*target_event_ids_count + 1) * sizeof(int32_t));

    if (new_event_ids == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    target_event_ids = new_event_ids;
    target_event_ids[*target_event_ids_count] = event_id;
    (*target_event_ids_count)++;

    if (event_t == MARKER_MODE_EVENT_ENTER)
    {
        mode->config.enter_event_ids = target_event_ids;
    }
    else if (event_t == MARKER_MODE_EVENT_EXIT)
    {
        mode->config.exit_event_ids = target_event_ids;
    }

    return ESP_OK;
}

esp_err_t marker_mode_remove_event(int32_t event_id, enmr_marker_mode_t *mode, enmr_marker_mode_event_t event_t)
{
    int32_t *target_event_ids;
    int32_t *target_event_ids_count;

    if (event_t == MARKER_MODE_EVENT_ENTER)
    {
        target_event_ids = mode->config.enter_event_ids;
        target_event_ids_count = &mode->config.enter_event_ids_count;
    }
    else if (event_t == MARKER_MODE_EVENT_EXIT)
    {
        target_event_ids = mode->config.exit_event_ids;
        target_event_ids_count = &mode->config.exit_event_ids_count;
    }
    else
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (*target_event_ids_count == 0)
    {
        return ESP_ERR_NOT_FOUND;
    }

    for (int i = 0; i < *target_event_ids_count; i++)
    {
        if (event_id == target_event_ids[i])
        {
            for (int j = i; j < *target_event_ids_count - 1; j++)
            {
                target_event_ids[j] = target_event_ids[j + 1];
            }

            int32_t *new_event_ids = realloc(target_event_ids, (*target_event_ids_count - 1) * sizeof(int32_t));

            if (new_event_ids == NULL)
            {
                return ESP_ERR_NO_MEM;
            }

            target_event_ids = new_event_ids;
            (*target_event_ids_count)--;

            if (event_t == MARKER_MODE_EVENT_ENTER)
            {
                mode->config.enter_event_ids = target_event_ids;
            }
            else if (event_t == MARKER_MODE_EVENT_EXIT)
            {
                mode->config.exit_event_ids = target_event_ids;
            }

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}
