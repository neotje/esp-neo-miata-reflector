#include "mode_event_linker.h"

ESP_EVENT_DEFINE_BASE(MODE_EVENT);

/*
PRIVATE
*/
static const char* TAG = "mode_event_linker";

mode_event_link_t *mode_event_links = NULL;
uint32_t mode_event_links_count = 0;

esp_event_loop_handle_t mode_event_linker_event_loop;

void mode_event_linker_handler(void* args, esp_event_base_t base, int32_t id, void* event_data)
{
    esp_err_t err;

    for (uint32_t i = 0; i < mode_event_links_count; i++)
    {
        if (mode_event_links[i].event_id == id)
        {
            switch (mode_event_links[i].action)
            {
            case ENTER_ACTION:
                err = mode_stack_manager_enter_mode(mode_event_links[i].mode_id);

                if (err != ESP_OK) ESP_LOGE(TAG, "Failed to enter mode %d", mode_event_links[i].mode_id);
                break;

            case EXIT_ACTION:
                err = mode_stack_manager_exit_mode(mode_event_links[i].mode_id);

                if (err != ESP_OK) ESP_LOGE(TAG, "Failed to exit mode %d", mode_event_links[i].mode_id);
                break;
            
            default:
                ESP_LOGW(TAG, "Invalid action %d", mode_event_links[i].action);
                break;
            }
        }
    }
}

esp_err_t mode_event_linker_load()
{
    mode_event_link_t *temp = mode_event_links;

    // Get the size of the links blob
    size_t required_size = 0;
    ESP_RETURN_ON_ERROR(config_manager_get_blob("modeEventLinker", "links", NULL, &required_size), TAG, "Failed to get mode event links size");

    ESP_RETURN_ON_FALSE(required_size > 0, ESP_OK, TAG, "Nothing to load");

    // Allocate or reallocate memory for the links blob
    if (temp == NULL)
    {
        temp = malloc(required_size);
    }
    else
    {
        temp = realloc(temp, required_size);
    }

    ESP_RETURN_ON_FALSE(temp != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for mode event links");

    // Get the links blob
    ESP_RETURN_ON_ERROR(config_manager_get_blob("modeEventLinker", "links", temp, &required_size), TAG, "Failed to get mode event links");

    // Update the links
    mode_event_links = temp;
    mode_event_links_count = required_size / sizeof(mode_event_link_t);

    return ESP_OK;
}

esp_err_t mode_event_linker_save()
{
    // Save the links blob
    ESP_RETURN_ON_ERROR(config_manager_set_blob("modeEventLinker", "links", mode_event_links, mode_event_links_count * sizeof(mode_event_link_t)), TAG, "Failed to set mode event links");

    return ESP_OK;
}

/*
PUBLIC
*/
esp_err_t mode_event_linker_init()
{
    ESP_LOGI(TAG, "Initializing mode event linker");

    // Create the event loop
    esp_event_loop_args_t event_loop_args = {
        .queue_size = CONFIG_MODE_EVENT_LINKER_EVENT_QUEUE_SIZE,
        .task_name = "mode_event_linker_event_loop",
        .task_priority = uxTaskPriorityGet(NULL) + CONFIG_MODE_EVENT_LINKER_EVENT_LOOP_PRIORITY,
        .task_stack_size = CONFIG_MODE_EVENT_LINKER_EVENT_LOOP_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY
    };

    ESP_RETURN_ON_ERROR(esp_event_loop_create(&event_loop_args, &mode_event_linker_event_loop), TAG, "Failed to create mode event linker event loop");

    // Set up the event handler
    ESP_RETURN_ON_ERROR(esp_event_handler_register_with(mode_event_linker_event_loop, MODE_EVENT, ESP_EVENT_ANY_ID, mode_event_linker_handler, NULL), TAG, "Failed to register mode event linker handler");

    // Load the mode event links
    ESP_RETURN_ON_ERROR(mode_event_linker_load(), TAG, "Failed to load mode event links");

    ESP_LOGI(TAG, "Mode event linker initialized");

    return ESP_OK;
}

esp_err_t mode_event_linker_add(uint8_t mode_id, int32_t event_id, mode_event_link_action_t action)
{
    // check if mode_id is valid
    ESP_RETURN_ON_ERROR(mode_stack_manager_get_mode(mode_id, NULL), TAG, "Invalid mode id");

    // check if link already exists
    for (uint32_t i = 0; i < mode_event_links_count; i++)
    {
        if (mode_event_links[i].mode_id == mode_id && mode_event_links[i].event_id == event_id)
        {
            return ESP_ERR_INVALID_STATE;
        }
    }

    // Add the link
    mode_event_link_t *temp = mode_event_links;

    if (mode_event_links == NULL)
    {
        temp = malloc(sizeof(mode_event_link_t));
    }
    else
    {
        temp = realloc(mode_event_links, (mode_event_links_count + 1) * sizeof(mode_event_link_t));
    }

    ESP_RETURN_ON_FALSE(temp != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for mode event link");

    temp[mode_event_links_count].mode_id = mode_id;
    temp[mode_event_links_count].event_id = event_id;
    temp[mode_event_links_count].action = action;

    mode_event_links = temp;
    mode_event_links_count++;

    // Save the links
    if (mode_event_linker_save() != ESP_OK) return ESP_FAIL;

    return ESP_OK;
}

esp_err_t mode_event_linker_remove(uint8_t mode_id, int32_t event_id, mode_event_link_action_t action)
{
    // check if link exists
    for (uint32_t i = 0; i < mode_event_links_count; i++)
    {
        if (mode_event_links[i].mode_id == mode_id && mode_event_links[i].event_id == event_id && mode_event_links[i].action == action)
        {
            // Remove the link
            mode_event_link_t *temp = mode_event_links;

            if (mode_event_links_count == 1)
            {
                free(temp);
                mode_event_links = NULL;
            }
            else
            {
                temp = malloc((mode_event_links_count - 1) * sizeof(mode_event_link_t));

                ESP_RETURN_ON_FALSE(temp != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for mode event link");

                for (uint32_t j = 0, k = 0; j < mode_event_links_count; j++)
                {
                    if (j != i)
                    {
                        temp[k] = mode_event_links[j];
                        k++;
                    }
                }

                free(mode_event_links);
                mode_event_links = temp;
            }

            mode_event_links_count--;

            // Save the links
            if (mode_event_linker_save() != ESP_OK) return ESP_FAIL;

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

esp_err_t mode_event_linker_post(int32_t event_id, const void *event_data, size_t event_data_size)
{
    // Post the event
    return esp_event_post_to(mode_event_linker_event_loop, MODE_EVENT, event_id, event_data, event_data_size, portMAX_DELAY);
}

void mode_event_linker_print()
{
    ESP_LOGI(TAG, "Mode event links:");

    for (uint32_t i = 0; i < mode_event_links_count; i++)
    {
        ESP_LOGI(TAG, "Mode %d, Event %d, Action %d", (int)mode_event_links[i].mode_id, (int)mode_event_links[i].event_id, (int)mode_event_links[i].action);
    }
}