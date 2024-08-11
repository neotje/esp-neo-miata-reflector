#include <stdio.h>

#include "esp_log.h"
#include "esp_console.h"

#include "config_manager.h"
#include "state_manager.h"
#include "mode_stack_manager.h"
#include "mode_event_linker.h"
#include "sense_wire_event_source.h"
#include "blinker_event_source.h"
#include "gfx.h"
#include "marker_blinking_mode.h"
#include "blinker_emulator.h"

static const char *TAG = "marker_app_main";

static void idle_mode_enter()
{
    ESP_LOGI(TAG, "Entering idle mode");
    gfx_clear();
}

stack_manager_mode_t idle_mode = {
    .id = 0,
    .name = "idle",
    .priority = IDLE_MODE_PRIORITY,
    .mode_enter_func = &idle_mode_enter,
    
};

static esp_console_repl_t *repl = NULL;
esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

esp_err_t init_console()
{
    repl_config.prompt = ">";
    repl_config.max_cmdline_length = 1024;

    ESP_RETURN_ON_ERROR(esp_console_register_help_command(), TAG, "register help command");

    register_restart();

    return ESP_OK;
}

esp_err_t start_console()
{
#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_console_new_repl_uart(&hw_config, &repl_config, &repl), TAG, "new repl uart failed");

#elif defined(CONFIG_ESP_CONSOLE_USB_CDC)
    esp_console_dev_usb_cdc_config_t hw_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_console_new_repl_usb_cdc(&hw_config, &repl_config, &repl), TAG, "new repl usb cdc failed");

#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl), TAG, "new repl usb serial jtag failed");

#endif

    return esp_console_start_repl(repl);
}

void app_main(void)
{
    ESP_ERROR_CHECK(init_console());

    ESP_ERROR_CHECK(config_manager_init());

    ESP_ERROR_CHECK(state_manager_init());

    ESP_ERROR_CHECK(mode_stack_manager_init(&idle_mode));

    ESP_ERROR_CHECK(marker_blinking_mode_init());

    ESP_ERROR_CHECK(mode_event_linker_init());

    ESP_ERROR_CHECK(sense_wire_event_source_init());

    ESP_ERROR_CHECK(blinker_event_source_init());

    ESP_ERROR_CHECK(gfx_init());

    ESP_ERROR_CHECK(blinker_emulator_init());

    ESP_ERROR_CHECK(start_console());
}
