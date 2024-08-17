/**
 * @file daytime_running_mode.c
 * @version 1.0
 */

#include "daytime_running_mode.h"

/*
PRIVATE
*/
static const char *TAG = "marker_blinking_mode";
static const char *NAMESPACE = "runningMode";
static const char *ON_COLOR_KEY = "color";

uint32_t running_color = 0xffffff;

static void daytime_running_config_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    config_manager_event_update_t *update = (config_manager_event_update_t *)event_data;

    if (strcmp(update->namespace, NAMESPACE) == 0 && id == CONFIG_MANAGER_EVENT_UPDATE)
    {
        if (strcmp(update->key, ON_COLOR_KEY) == 0)
        {
            config_manager_get_u32(NAMESPACE, ON_COLOR_KEY, &running_color);
        }
    }
}

static void daytime_running_mode_enter(void *args)
{
    ESP_LOGI(TAG, "Entered daytime_running_mode");
    gfx_start_transition();
}

static void daytime_running_mode_function(void *args)
{
    gfx_draw_line(running_color, 0, gfx_get_length());
}

static void daytime_running_mode_exit(void *args)
{
    ESP_LOGI(TAG, "Exited daytime_running_mode");
}

stack_manager_mode_t daytime_running_mode = {
    .id = CONFIG_DAYTIME_RUNNING_MODE_ID,
    .name = "daytime_running_mode",
    .priority = CONFIG_DAYTIME_RUNNING_MODE_PRIO,
    .mode_enter_func = &daytime_running_mode_enter,
    .mode_function = &daytime_running_mode_function,
    .mode_exit_func = &daytime_running_mode_exit,
};

static struct {
    struct arg_str *cmd;
    struct arg_str *action;
    struct arg_int *value;
    struct arg_end *end;
} running_mode_cmd_args;

static int running_mode_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&running_mode_cmd_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, running_mode_cmd_args.end, argv[0]);
        return 1;
    }

    if (strcmp(running_mode_cmd_args.cmd->sval[0], "color") == 0)
    {
        if (strcmp(running_mode_cmd_args.action->sval[0], "set") == 0)
        {
            ESP_RETURN_ON_ERROR(config_manager_set_u32(NAMESPACE, ON_COLOR_KEY, (uint32_t)running_mode_cmd_args.value->ival), TAG, "Failed to set on_color");
        }
        else if (strcmp(running_mode_cmd_args.action->sval[0], "get") == 0)
        {
            ESP_LOGI(TAG, "Current color: 0x%06lx", running_color);
        }
    }

    return 0;
}

esp_err_t daytime_running_mode_register_running_mode_cmd()
{
    running_mode_cmd_args.cmd = arg_str1(NULL, NULL, "<cmd>", "Command to run: color");
    running_mode_cmd_args.action = arg_str1(NULL, NULL, "<action>", "Action to take: set or get");
    running_mode_cmd_args.value = arg_int0(NULL, NULL, "<value>", "Value to set");
    running_mode_cmd_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "running_mode",
        .help = "Run a command on the running mode",
        .hint = NULL,
        .func = &running_mode_cmd,
        .argtable = &running_mode_cmd_args,
    };

    ESP_RETURN_ON_ERROR(esp_console_cmd_register(&cmd), TAG, "Failed to register running_mode command");

    return ESP_OK;
}

/*
PUBLIC
*/
esp_err_t daytime_running_mode_init()
{
    ESP_LOGI(TAG, "Initializing daytime_running_mode");

    ESP_RETURN_ON_ERROR(mode_stack_manager_add_mode(&daytime_running_mode), TAG, "Failed to add daytime_running_mode to mode stack");

    ESP_RETURN_ON_ERROR(config_manager_get_u32(NAMESPACE, ON_COLOR_KEY, &running_color), TAG, "Failed to get on_color from config manager");

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(&daytime_running_config_handler, NULL), TAG, "Failed to register config handler");

    ESP_RETURN_ON_ERROR(daytime_running_mode_register_running_mode_cmd(), TAG, "Failed to register running_mode command");

    mode_event_linker_add(CONFIG_DAYTIME_RUNNING_MODE_ID, MARKER_EVENT_SENSE1_ON, ENTER_ACTION);
    mode_event_linker_add(CONFIG_DAYTIME_RUNNING_MODE_ID, MARKER_EVENT_SENSE1_OFF, EXIT_ACTION);

    return ESP_OK;
}
