#include "blinker_emulator.h"

/*
PRIVATE
*/
static const char* TAG = "blinker_emulator";

uint64_t on_time = 0;
uint64_t off_time = 0;
uint8_t sense_wire_index = 0;
int times = 0;
int current_times = 0;
bool is_on = false;

esp_timer_handle_t emulator_timer;

static struct {
    struct arg_str *wire;
    struct arg_int *on_ms;
    struct arg_int *off_ms;
    struct arg_int *times;
    struct arg_end *end;
} start_blinker_args;

static void blinker_emulator_timer(void *args)
{
    sense_wire_t *sense_wire = get_sense_wire(sense_wire_index);
    int64_t time = esp_timer_get_time();

    if (is_on)
    {
        mode_event_linker_post(sense_wire->off_event_id, (void*)&time, sizeof(int64_t));
        current_times++;

        if (current_times >= times)
        {
            ESP_LOGI(TAG, "Blinker finished");

            esp_timer_stop(emulator_timer);
            esp_timer_delete(emulator_timer);
            emulator_timer = NULL;

            return;
        }

        ESP_ERROR_CHECK(esp_timer_start_once(emulator_timer, off_time));
    } 
    else
    {
        mode_event_linker_post(sense_wire->on_event_id, (void*)&time, sizeof(int64_t));
        ESP_ERROR_CHECK(esp_timer_start_once(emulator_timer, on_time));
    }

    is_on = !is_on;
}

static int blinker_emulator_start_trigger(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &start_blinker_args);

    if (nerrors != 0)
    {
        arg_print_errors(stderr, start_blinker_args.end, argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    if (emulator_timer != NULL)
    {
        ESP_LOGI(TAG, "Stopping previous blinker");
        esp_timer_stop(emulator_timer);
        esp_timer_delete(emulator_timer);

        emulator_timer = NULL;
    }

    if (strcmp(start_blinker_args.wire->sval[0], "SENSE1") == 0)
    {
        sense_wire_index = 0;
    }
    else if (strcmp(start_blinker_args.wire->sval[0], "SENSE2") == 0)
    {
        sense_wire_index = 1;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid wire name: %s", start_blinker_args.wire->sval[0]);
        return ESP_ERR_INVALID_ARG;
    }

    if (start_blinker_args.on_ms->ival[0] < 10)
    {
        ESP_LOGE(TAG, "On time must be greater than 10");
        return ESP_ERR_INVALID_ARG;
    }

    if (start_blinker_args.off_ms->ival[0] < 10)
    {
        ESP_LOGE(TAG, "Off time must be greater than 10");
        return ESP_ERR_INVALID_ARG;
    }

    on_time = start_blinker_args.on_ms->ival[0] * 1000;
    off_time = start_blinker_args.off_ms->ival[0] * 1000;
    is_on = false;
    times = start_blinker_args.times->ival[0];
    current_times = 0;

    esp_timer_create_args_t timer_args = {
        .callback = &blinker_emulator_timer,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "blinker_emulator_timer"
    };

    ESP_RETURN_ON_ERROR(esp_timer_create(&timer_args, &emulator_timer), TAG, "Failed to create timer");

    blinker_emulator_timer(NULL);

    return 0;
}

esp_err_t blinker_emulator_register_start_blinker() 
{
    start_blinker_args.wire = arg_str1(NULL, NULL, "<wire>", "Wire name: SENSE1 or SENSE2");
    start_blinker_args.on_ms = arg_int1(NULL, NULL, "<on_ms>", "On time in ms");
    start_blinker_args.off_ms = arg_int1(NULL, NULL, "<off_ms>", "Off time in ms");
    start_blinker_args.times = arg_int1(NULL, NULL, "<times>", "Number of times to blink");
    start_blinker_args.end = arg_end(4);

    const esp_console_cmd_t cmd = {
        .command = "start_blinker",
        .help = "Emulates a blinker by triggering sense wire events",
        .hint = NULL,
        .func = &blinker_emulator_start_trigger,
        .argtable = &start_blinker_args
    };

    ESP_RETURN_ON_ERROR(esp_console_cmd_register(&cmd), TAG, "Failed to register command");

    return ESP_OK;
}

/*
PUBLIC
*/

esp_err_t blinker_emulator_init()
{
    ESP_RETURN_ON_ERROR(blinker_emulator_register_start_blinker(), TAG, "Failed to register start_blinker command");

    return ESP_OK;
}
