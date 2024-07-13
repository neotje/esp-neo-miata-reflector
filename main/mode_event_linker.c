/**
 * @file mode_event_linker.c
 * @version 1.1
 */

#include "mode_event_linker.h"

ESP_EVENT_DEFINE_BASE(MODE_EVENT);

/*
PRIVATE
*/
static const char* TAG = "mode_event_linker";

mode_event_link_t *mode_event_links = NULL;
uint32_t mode_event_links_count = 0;

esp_event_loop_handle_t mode_event_linker_event_loop;

esp_err_t mode_event_linker_load();

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

void mode_event_linker_config_update_handler(void* args, esp_event_base_t base, int32_t id, void* event_data)
{
    config_manager_event_update_t* update = (config_manager_event_update_t*)event_data;
    
    if (id == CONFIG_MANAGER_EVENT_UPDATE && strcmp(update->namespace, "modeEventLinker") == 0 && strcmp(update->key, "links") == 0)
    {
        ESP_LOGI(TAG, "Reloading mode event links");
        mode_event_linker_load();
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

static struct {
    struct arg_str *event;
    struct arg_end *end;
} trigger_args;

static int mode_event_linker_trigger(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &trigger_args);

    if (nerrors != 0)
    {
        arg_print_errors(stderr, trigger_args.end, argv[0]);
        return 1;
    }

    int32_t event_id = -1;
    void *event_data = NULL;
    size_t event_data_size = 0;

    if (strcmp(trigger_args.event->sval[0], "SENSE1_ON") == 0)
    {
        event_id = MARKER_EVENT_SENSE1_ON;
        int64_t time = esp_timer_get_time();
        event_data = &time;
        event_data_size = sizeof(time);
    }
    else if (strcmp(trigger_args.event->sval[0], "SENSE2_ON") == 0)
    {
        event_id = MARKER_EVENT_SENSE2_ON;
        int64_t time = esp_timer_get_time();
        event_data = &time;
        event_data_size = sizeof(time);
    }
    else if (strcmp(trigger_args.event->sval[0], "SENSE1_OFF") == 0)
    {
        event_id = MARKER_EVENT_SENSE1_OFF;
        int64_t time = esp_timer_get_time();
        event_data = &time;
        event_data_size = sizeof(time);
    }
    else if (strcmp(trigger_args.event->sval[0], "SENSE2_OFF") == 0)
    {
        event_id = MARKER_EVENT_SENSE2_OFF;
        int64_t time = esp_timer_get_time();
        event_data = &time;
        event_data_size = sizeof(time);
    }
    else if (strcmp(trigger_args.event->sval[0], "RUNNING_LIGHTS_ON") == 0)
    {
        event_id = MARKER_EVENT_RUNNING_LIGHTS_ON;
    }
    else if (strcmp(trigger_args.event->sval[0], "RUNNING_LIGHTS_OFF") == 0)
    {
        event_id = MARKER_EVENT_RUNNING_LIGHTS_OFF;
    }
    else if (strcmp(trigger_args.event->sval[0], "TURN_SIGNAL_ON") == 0)
    {
        event_id = MARKER_EVENT_TURN_SIGNAL_ON;
    }
    else if (strcmp(trigger_args.event->sval[0], "TURN_SIGNAL_OFF") == 0)
    {
        event_id = MARKER_EVENT_TURN_SIGNAL_OFF;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid event %s", trigger_args.event->sval[0]);
        return 1;
    }

    esp_err_t result = mode_event_linker_post(event_id, event_data, event_data_size);

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to post event %s", trigger_args.event->sval[0]);
        return 1;
    }

    return 0;
}

esp_err_t mode_event_linker_register_trigger()
{
    trigger_args.event = arg_str1(NULL, NULL, "<event>", "Event to trigger: SENSE1_ON, SENSE2_ON, SENSE1_OFF, SENSE2_OFF, RUNNING_LIGHTS_ON, RUNNING_LIGHTS_OFF, TURN_SIGNAL_ON, TURN_SIGNAL_OFF");
    trigger_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "trigger",
        .help = "Trigger an event",
        .hint = NULL,
        .func = &mode_event_linker_trigger,
        .argtable = &trigger_args
    };

    ESP_RETURN_ON_ERROR(esp_console_cmd_register(&cmd), TAG, "Failed to register trigger command");

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

    // Set up the config update handler
    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(&mode_event_linker_config_update_handler, NULL), TAG, "Failed to register config update handler");

    // Load the mode event links
    ESP_RETURN_ON_ERROR(mode_event_linker_load(), TAG, "Failed to load mode event links");

    ESP_RETURN_ON_ERROR(mode_event_linker_register_trigger(), TAG, "Failed to register trigger command");

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
    ESP_LOGI(TAG, "Posting event %s", mode_event_to_string(event_id));
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

esp_err_t mode_event_linker_register_handler(int32_t event_id, esp_event_handler_t event_handler, void* evnent_handler_arg, esp_event_handler_instance_t* instance)
{
    return esp_event_handler_instance_register_with(mode_event_linker_event_loop, MODE_EVENT, event_id, event_handler, evnent_handler_arg, instance);
}

esp_err_t mode_event_linker_unregister_handler(int32_t event_id, esp_event_handler_instance_t instance)
{
    return esp_event_handler_instance_unregister_with(mode_event_linker_event_loop, MODE_EVENT, event_id, instance);
}