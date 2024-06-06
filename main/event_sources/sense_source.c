#include "sense_source.h"

static const char *TAG = "sense_source";

adc_oneshot_unit_handle_t adc_handle;
adc_channel_t sense1_channel = 0;
adc_channel_t sense2_channel = 0;

int states[2] = {0, 0};
uint32_t thresholds[2] = {0, 0};

TaskHandle_t sense_source_task_handle;

static void sense_source_task(void *arg)
{
    ESP_LOGI(TAG, "Sense source task started");

    nvs_handle_t nvs_handle;
    nvs_open("sense", NVS_READONLY, &nvs_handle);

    while (1)
    {
        for (int i = 0; i < 2; i++)
        {
            int previous_state = states[i];

            if(nvs_get_u32(nvs_handle, i == 0 ? "threshold1" : "threshold2", &thresholds[i]) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read threshold from NVS");
                continue;
            }

            int value;
            if (adc_oneshot_read(adc_handle, i == 0 ? sense1_channel : sense2_channel, &value) != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to read ADC value");
                continue;
            }

            if (value > thresholds[i] && states[i] == 0)
            {
                states[i] = 1;
                ESP_LOGI(TAG, "Threshold %d exceeded", i + 1);
            }
            else if (value <= thresholds[i] && states[i] == 1)
            {
                states[i] = 0;
                ESP_LOGI(TAG, "Threshold %d not exceeded", i + 1);
            }

            if (previous_state == 0 && states[i] == 1)
            {
                if (marker_event_loop_trigger(MARKER_EVENT_SENSE1_ON + i, &value, sizeof(value), portMAX_DELAY) != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to trigger marker event");
                }
            }
            else if (previous_state == 1 && states[i] == 0)
            {
                if (marker_event_loop_trigger(MARKER_EVENT_SENSE1_OFF + i, &value, sizeof(value), portMAX_DELAY) != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to trigger marker event");
                }
            }
        }

        vTaskDelay(10);
    }
}

esp_err_t sense_source_init()
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&init_config, &adc_handle), TAG, "Failed to create ADC unit");

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    // adc_continuous_io_to_channel
    // https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32s3/api-reference/peripherals/adc_continuous.html#_CPPv428adc_continuous_io_to_channeliPC10adc_unit_tPC13adc_channel_t

    adc_unit_t sense1_adc_unit = ADC_UNIT_1;
    ESP_RETURN_ON_ERROR(adc_continuous_io_to_channel(CONFIG_BOARD_SENS1_PIN, &sense1_adc_unit, &sense1_channel), TAG, "Failed to convert IO to channel");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, sense1_channel, &config), TAG, "Failed to configure channel");

    adc_unit_t sense2_adc_unit = ADC_UNIT_1;
    ESP_RETURN_ON_ERROR(adc_continuous_io_to_channel(CONFIG_BOARD_SENS2_PIN, &sense2_adc_unit, &sense2_channel), TAG, "Failed to convert IO to channel");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, sense2_channel, &config), TAG, "Failed to configure channel");

    ESP_LOGI(TAG, "ADC unit initialized");

    xTaskCreate(sense_source_task, "sense_source_task", 3072, NULL, uxTaskPriorityGet(NULL) + CONFIG_MARKER_EVENT_LOOP_TASK_PRIORITY, &sense_source_task_handle);

    return ESP_OK;
}

esp_err_t sense_source_deinit()
{
    return adc_oneshot_del_unit(adc_handle);
}