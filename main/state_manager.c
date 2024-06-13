#include "state_manager.h"

/*
PRIVATE
*/
ESP_EVENT_DEFINE_BASE(STATE_MANAGER_EVENT);

static const char *TAG = "state_manager";

state_entry_t **state_entries;
size_t state_entries_size = 0;

esp_event_loop_handle_t state_manager_event_loop;

esp_err_t trigger_update_event(const char *key)
{
    return esp_event_post_to(state_manager_event_loop, STATE_MANAGER_EVENT, STATE_MANAGER_EVENT_UPDATE, (void *)key, strlen(key), portMAX_DELAY);
}

esp_err_t get_entry_by_key(const char* key, state_entry_t** entry)
{
    for (size_t i = 0; i < state_entries_size; i++)
    {
        if (strcmp(state_entries[i]->key, key) == 0)
        {
            *entry = state_entries[i];
            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

/*
PUBLIC
*/
esp_err_t state_manager_init()
{
    ESP_LOGI(TAG, "Initializing state manager");

    state_entries = NULL;
    state_entries_size = 0;

    esp_event_loop_args_t loop_args = {
        .queue_size = CONFIG_STATE_MANAGER_EVENT_QUEUE_SIZE,
        .task_name = "state_manager_event_loop",
        .task_priority = uxTaskPriorityGet(NULL) + CONFIG_STATE_MANAGER_EVENT_LOOP_PRIORITY,
        .task_stack_size = CONFIG_STATE_MANAGER_EVENT_LOOP_STACK_SIZE,
        .task_core_id = tskNO_AFFINITY};

    esp_err_t err = esp_event_loop_create(&loop_args, &state_manager_event_loop);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to create state manager event loop");

    ESP_LOGI(TAG, "State manager initialized");

    return ESP_OK;
}

esp_err_t state_manager_set(const char *key, void *value, size_t size)
{
    state_entry_t *entry;

    // check if entry already exists and update it
    if (get_entry_by_key(key, &entry) == ESP_OK)
    {
        // copy new value to entry and reallocate memory if needed
        if (entry->size != size)
        {
            entry->value = realloc(entry->value, size);

            ESP_RETURN_ON_FALSE(entry->value != NULL, ESP_ERR_NO_MEM, TAG, "Failed to reallocate memory for state entry value");
        }

        memcpy(entry->value, value, size);
        entry->size = size;

        ESP_RETURN_ON_ERROR(trigger_update_event(key), TAG, "Failed to trigger update event");

        return ESP_OK;
    }

    // create new entry
    state_entry_t **temp_entries = state_entries;

    if (temp_entries == NULL || state_entries_size == 0)
    {
        temp_entries = malloc(sizeof(state_entry_t *));
    }
    else
    {
        temp_entries = realloc(state_entries, sizeof(state_entry_t *) * (state_entries_size + 1));
    }

    ESP_RETURN_ON_FALSE(temp_entries != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for state entry");

    entry = malloc(sizeof(state_entry_t));

    ESP_RETURN_ON_FALSE(entry != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for state entry");

    // copy value to entry
    entry->value = malloc(size);

    ESP_RETURN_ON_FALSE(entry->value != NULL, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for state entry value");

    memcpy(entry->value, value, size);

    entry->key = key;
    entry->size = size;

    temp_entries[state_entries_size] = entry;

    state_entries = temp_entries;

    state_entries_size++;

    ESP_RETURN_ON_ERROR(trigger_update_event(key), TAG, "Failed to trigger update event");

    return ESP_OK;
}

esp_err_t state_manager_get(const char *key, void **out, size_t *out_size)
{
    state_entry_t *entry;

    ESP_RETURN_ON_ERROR(get_entry_by_key(key, &entry), TAG, "Failed to get state entry by key");

    *out = entry->value;
    *out_size = entry->size;

    return ESP_OK;
}

esp_err_t state_manager_register_update_handler(esp_event_handler_t event_handler, esp_event_handler_instance_t *instance)
{
    return esp_event_handler_instance_register_with(
        state_manager_event_loop,
        STATE_MANAGER_EVENT,
        STATE_MANAGER_EVENT_UPDATE,
        event_handler,
        NULL,
        instance
    );
}

esp_err_t state_manager_unregister_update_handler(esp_event_handler_instance_t instance)
{
    return esp_event_handler_instance_unregister_with(
        state_manager_event_loop,
        STATE_MANAGER_EVENT,
        STATE_MANAGER_EVENT_UPDATE,
        instance
    );
}

void state_manager_print_entries()
{
    ESP_LOGI(TAG, "State entries:");

    for (size_t i = 0; i < state_entries_size; i++)
    {
        ESP_LOGI(TAG, "Key: %s", state_entries[i]->key);
    }
}