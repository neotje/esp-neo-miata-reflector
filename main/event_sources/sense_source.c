#include "sense_source.h"

static const char* TAG = "sense_source";

adc_oneshot_unit_handle_t adc_handle;

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

    adc_channel_t sense1_channel = 0;
    adc_unit_t sense1_adc_unit = ADC_UNIT_1;
    ESP_RETURN_ON_ERROR(adc_continuous_io_to_channel(CONFIG_BOARD_SENS1_PIN, &sense1_adc_unit, &sense1_channel), TAG, "Failed to convert IO to channel");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, sense1_channel, &config), TAG, "Failed to configure channel");


    adc_channel_t sense2_channel = 0;
    adc_unit_t sense2_adc_unit = ADC_UNIT_1;
    ESP_RETURN_ON_ERROR(adc_continuous_io_to_channel(CONFIG_BOARD_SENS2_PIN, &sense2_adc_unit, &sense2_channel), TAG, "Failed to convert IO to channel");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, sense2_channel, &config), TAG, "Failed to configure channel");

    ESP_LOGI(TAG, "ADC unit initialized");

    return ESP_OK;
}

esp_err_t sense_source_deinit()
{
    return adc_oneshot_del_unit(adc_handle);
}