/**
 * @file blinker_event_source.c
 * @version 1.0
 */

#include "blinker_event_source.h"

/*
PRIVATE
*/
static const char *TAG = "blinker_event_source";
static const char *NAMESPACE = "blinker";
static const char *ON_DURATION_KEY = "onDuration";
static const char *OFF_DURATION_KEY = "offDuration";
static const char *TIMER_OFFSET_KEY = "timerOffset";
static const char *SENSE_WIRE_INDEX_KEY = "sense";

uint8_t current_sense_wire_index = 0;
esp_timer_handle_t off_timer;
esp_event_handler_instance_t event_handler_instance;

int64_t last_on_time = -1;
int64_t last_off_time = -1;

int64_t on_duration = 0;
int64_t off_duration = 0;
int64_t timer_offset = 0;

void calculateOffDuration();
void calculateOnDuration();

static void blinker_event_source_config_update_handler(void *args, esp_event_base_t base, int32_t id, void *event_data)
{
    config_manager_event_update_t *config_event = (config_manager_event_update_t *)event_data;

    if (strcmp(config_event->namespace, NAMESPACE) != 0)
        return;

    if (strcmp(config_event->key, ON_DURATION_KEY) == 0)
    {
        config_manager_get_i64(config_event->namespace, config_event->key, &on_duration);
    }
    else if (strcmp(config_event->key, OFF_DURATION_KEY) == 0)
    {
        config_manager_get_i64(config_event->namespace, config_event->key, &off_duration);
    }
    else if (strcmp(config_event->key, TIMER_OFFSET_KEY) == 0)
    {
        config_manager_get_i64(config_event->namespace, config_event->key, &timer_offset);
    }
    else if (strcmp(config_event->key, SENSE_WIRE_INDEX_KEY) == 0)
    {
        uint8_t sense_wire_index = 0;
        config_manager_get_u8(config_event->namespace, config_event->key, &sense_wire_index);
        blinker_event_source_set_sense_wire(sense_wire_index);
    }
}

static void blinker_event_source_off_timer(void *args)
{
    mode_event_linker_post(MARKER_EVENT_TURN_SIGNAL_OFF, NULL, 0);
}

static void blinker_event_source_on_sense_wire_event(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    int64_t *time = (int64_t *)event_data;

    sense_wire_t *sense_wire = get_sense_wire(current_sense_wire_index - 1);

    if (sense_wire == NULL)
    {
        ESP_LOGW(TAG, "Sense wire %d not found", current_sense_wire_index - 1);
        return;
    }

    if (id == sense_wire->on_event_id)
    {
        last_on_time = *time;

        esp_timer_stop(off_timer);

        calculateOffDuration();

        mode_event_linker_post(MARKER_EVENT_TURN_SIGNAL_ON, NULL, 0);
    }

    if (id == sense_wire->off_event_id)
    {
        last_off_time = *time;

        calculateOnDuration();

        esp_timer_start_once(off_timer, off_duration + timer_offset);
    }
}

esp_err_t unregister_wire(uint8_t index)
{
    sense_wire_t *sense_wire = get_sense_wire(index);

    ESP_RETURN_ON_FALSE(sense_wire != NULL, ESP_ERR_INVALID_STATE, TAG, "Sense wire %d not found", index);

    ESP_RETURN_ON_ERROR(mode_event_linker_unregister_handler(sense_wire->on_event_id, event_handler_instance), TAG, "Failed to unregister on event handler");
    ESP_RETURN_ON_ERROR(mode_event_linker_unregister_handler(sense_wire->off_event_id, event_handler_instance), TAG, "Failed to unregister off event handler");

    return ESP_OK;
}

esp_err_t register_wire(uint8_t index)
{
    sense_wire_t *sense_wire = get_sense_wire(index);

    ESP_RETURN_ON_FALSE(sense_wire != NULL, ESP_ERR_INVALID_STATE, TAG, "Sense wire %d not found", index);

    ESP_RETURN_ON_ERROR(mode_event_linker_register_handler(sense_wire->on_event_id, blinker_event_source_on_sense_wire_event, NULL, &event_handler_instance), TAG, "Failed to register on event handler");
    ESP_RETURN_ON_ERROR(mode_event_linker_register_handler(sense_wire->off_event_id, blinker_event_source_on_sense_wire_event, NULL, &event_handler_instance), TAG, "Failed to register off event handler");

    return ESP_OK;
}

void calculateOffDuration()
{
    if (last_off_time != -1 && last_on_time != -1 && last_on_time > last_off_time)
    {
        off_duration = last_on_time - last_off_time;
        state_manager_set(OFF_DURATION_KEY, &off_duration, sizeof(off_duration));
        config_manager_set_i64(NAMESPACE, OFF_DURATION_KEY, off_duration);
        ESP_LOGI(TAG, "Off duration: %lld", off_duration / 1000);
    }
}

void calculateOnDuration()
{
    if (last_off_time != -1 && last_on_time != -1 && last_off_time > last_on_time)
    {
        on_duration = last_off_time - last_on_time;
        state_manager_set(ON_DURATION_KEY, &on_duration, sizeof(on_duration));
        config_manager_set_i64(NAMESPACE, ON_DURATION_KEY, on_duration);
        ESP_LOGI(TAG, "On duration: %lld", on_duration / 1000);
    }
}

static struct {
    struct arg_int *index;
    struct arg_end *end;
} set_sense_wire_args;

static int blinker_event_source_set_sense_wire_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&set_sense_wire_args);

    if (nerrors != 0)
    {
        arg_print_errors(stderr, set_sense_wire_args.end, argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    int index = set_sense_wire_args.index->ival[0];

    esp_err_t err = blinker_event_source_set_sense_wire(index);

    if (err != ESP_OK)
    {
        printf("\n");
        return 1;
    }

    err = config_manager_set_u8(NAMESPACE, SENSE_WIRE_INDEX_KEY, index);

    if (err != ESP_OK)
    {
        printf("\n");
        return 1;
    }

    return ESP_OK;
}

static esp_err_t blinker_event_source_register_set_sense_wire()
{
    set_sense_wire_args.index = arg_int1(NULL, NULL, "<index>", "0 - disable, 1 - sense wire 1, 2 - sense wire 2");
    set_sense_wire_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "set_sense_wire",
        .help = "Set the sense wire index for the blinker",
        .hint = NULL,
        .func = &blinker_event_source_set_sense_wire_cmd,
        .argtable = &set_sense_wire_args
    };

    ESP_RETURN_ON_ERROR(esp_console_cmd_register(&cmd), TAG, "Failed to register set_sense_wire command");

    return ESP_OK;
}

static struct {
    struct arg_end *end;
} get_blinker_args;

static int blinker_event_source_get_blinker_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&get_blinker_args);

    if (nerrors != 0)
    {
        arg_print_errors(stderr, get_blinker_args.end, argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    printf("On duration: %lld ms\n", on_duration / 1000);
    printf("Off duration: %lld ms\n", off_duration / 1000);
    printf("Timer offset: %lld\n", timer_offset);
    printf("Sense wire index: %d\n", current_sense_wire_index);

    return ESP_OK;
}

static esp_err_t blinker_event_source_register_get_blinker()
{
    get_blinker_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "get_blinker",
        .help = "Get the blinker configuration/measured values",
        .hint = NULL,
        .func = &blinker_event_source_get_blinker_cmd,
        .argtable = &get_blinker_args
    };

    ESP_RETURN_ON_ERROR(esp_console_cmd_register(&cmd), TAG, "Failed to register get_blinker command");

    return ESP_OK;

}

/*
PUBLIC
*/
esp_err_t blinker_event_source_init()
{
    ESP_LOGI(TAG, "Initializing blinker event source");

    const esp_timer_create_args_t off_timer_args = {
        .callback = &blinker_event_source_off_timer,
        .arg = NULL,
        .name = "blinker_event_source_off_timer"};

    ESP_RETURN_ON_ERROR(esp_timer_create(&off_timer_args, &off_timer), TAG, "Failed to create off timer");

    uint8_t sense_wire_index = 0;
    ESP_RETURN_ON_ERROR(config_manager_get_u8(NAMESPACE, SENSE_WIRE_INDEX_KEY, &sense_wire_index), TAG, "Failed to get sense wire index");
    ESP_RETURN_ON_ERROR(blinker_event_source_set_sense_wire(sense_wire_index), TAG, "Failed to set sense wire");

    // load config
    ESP_RETURN_ON_ERROR(config_manager_get_i64(NAMESPACE, ON_DURATION_KEY, &on_duration), TAG, "Failed to get on duration");
    ESP_RETURN_ON_ERROR(config_manager_get_i64(NAMESPACE, OFF_DURATION_KEY, &off_duration), TAG, "Failed to get off duration");
    ESP_RETURN_ON_ERROR(config_manager_get_i64(NAMESPACE, TIMER_OFFSET_KEY, &timer_offset), TAG, "Failed to get timer offset");

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(blinker_event_source_config_update_handler, NULL), TAG, "Failed to register config update handler");

    ESP_RETURN_ON_ERROR(blinker_event_source_register_set_sense_wire(), TAG, "Failed to register set_sense_wire command");
    ESP_RETURN_ON_ERROR(blinker_event_source_register_get_blinker(), TAG, "Failed to register get_blinker command");

    return ESP_OK;
}

esp_err_t blinker_event_source_set_sense_wire(int index)
{
    if (index == 0 && current_sense_wire_index != 0)
    {
        // deinit event handlers
        ESP_RETURN_ON_ERROR(unregister_wire(current_sense_wire_index - 1), TAG, "Failed to unregister sense wire");
    }
    else if (index != 0)
    {
        if (current_sense_wire_index != index && current_sense_wire_index != 0)
        {
            // deinit event handlers
            ESP_RETURN_ON_ERROR(unregister_wire(current_sense_wire_index - 1), TAG, "Failed to unregister sense wire");
        }

        // init event handlers
        ESP_RETURN_ON_ERROR(register_wire(index - 1), TAG, "Failed to register sense wire");
    }

    current_sense_wire_index = index;

    return ESP_OK;
}

int64_t blinker_event_source_get_on_duration()
{
    return on_duration;
}

int64_t blinker_event_source_get_off_duration()
{
    return off_duration;
}