#include "sense_wire_event_source.h"

/*
PRIVATE
*/
static const char *TAG = "sense_wire_event_source";

sense_wire_t sense_wires[] = {
    {
        .io_num = CONFIG_BOARD_SENS1_PIN,
        .adc_unit = ADC_UNIT_1,
        .adc_channel = 0,
        .threshold_key = "threshold1",
        .debounce_key = "debounce1",
        .threshold = 0,
        .debounce = 0,
        .on_event_id = MARKER_EVENT_SENSE1_ON,
        .off_event_id = MARKER_EVENT_SENSE1_OFF,
        .level = 0,
        .debounce_counter = 0,
    },
    {
        .io_num = CONFIG_BOARD_SENS2_PIN,
        .adc_unit = ADC_UNIT_1,
        .adc_channel = 0,
        .threshold_key = "threshold2",
        .debounce_key = "debounce2",
        .threshold = 0,
        .debounce = 0,
        .on_event_id = MARKER_EVENT_SENSE2_ON,
        .off_event_id = MARKER_EVENT_SENSE2_OFF,
        .level = 0,
        .debounce_counter = 0,
    }};

const size_t sense_wires_count = sizeof(sense_wires) / sizeof(sense_wire_t);

adc_oneshot_unit_handle_t adc_handle;

void sense_wire_event_source_config_update_handler(void *args, esp_event_base_t base, int32_t id, void *event_data)
{
    config_manager_event_update_t *config_event = (config_manager_event_update_t *)event_data;

    if (strcmp(config_event->namespace, "sense") != 0)
        return;

    for (size_t i = 0; i < sense_wires_count; i++)
    {
        if (strcmp(config_event->key, sense_wires[i].threshold_key) == 0)
        {
            config_manager_get_i32(config_event->namespace, config_event->key, &sense_wires[i].threshold);
        }
        else if (strcmp(config_event->key, sense_wires[i].debounce_key) == 0)
        {
            config_manager_get_i32(config_event->namespace, config_event->key, &sense_wires[i].debounce);
        }
    }
}

esp_err_t sense_wire_event_source_read_raw(int index, int *out_raw)
{
    if (index < 0 || index >= sense_wires_count)
    {
        ESP_LOGE(TAG, "Invalid sense wire index %d", index);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(adc_oneshot_read(adc_handle, sense_wires[index].adc_channel, out_raw), TAG, "Failed to read ADC value");

    return ESP_OK;
}

esp_err_t sense_wire_event_source_read_cali(int index, int *out_value)
{
    if (index < 0 || index >= sense_wires_count)
    {
        ESP_LOGE(TAG, "Invalid sense wire index %d", index);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(adc_oneshot_get_calibrated_result(
                            adc_handle,
                            sense_wires[index].cali_handle, 
                            sense_wires[index].adc_channel,
                            out_value),
                        TAG, "Failed to get calibrated ADC value");

    return ESP_OK;
}

/*
PUBLIC
*/
esp_err_t sense_wire_event_source_init()
{
    ESP_LOGI(TAG, "Initializing sense wire event source");

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&init_config, &adc_handle), TAG, "Failed to create ADC oneshot unit");

    for (size_t i = 0; i < sense_wires_count; i++)
    {
        ESP_LOGI(TAG, "Initializing sense wire %d", i);

        ESP_RETURN_ON_ERROR(adc_oneshot_io_to_channel(
                                sense_wires[i].io_num,
                                &sense_wires[i].adc_unit,
                                &sense_wires[i].adc_channel),
                            TAG, "Failed to convert IO to ADC channel");

        adc_oneshot_chan_cfg_t channel_config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(
                                adc_handle,
                                sense_wires[i].adc_channel,
                                &channel_config),
                            TAG, "Failed to configure ADC channel");

        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = sense_wires[i].adc_channel,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        esp_err_t err = adc_cali_create_scheme_curve_fitting(&cali_config, &sense_wires[i].cali_handle);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to create calibration scheme: %d", err);
        }
        else
        {
            ESP_LOGI(TAG, "Calibration scheme created");
        }

        // Read initial threshold and debounce values
        config_manager_get_i32("sense", sense_wires[i].threshold_key, &sense_wires[i].threshold);
        config_manager_get_i32("sense", sense_wires[i].debounce_key, &sense_wires[i].debounce);

        // test the sense wire
        int value = 0;
        ESP_RETURN_ON_ERROR(sense_wire_event_source_read_cali(i, &value), TAG, "Failed to read ADC value");

        ESP_LOGI(TAG, "Sense wire %d ADC value: %d", i, value);
    }

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(sense_wire_event_source_config_update_handler, NULL), TAG, "Failed to add config update handler");

    return ESP_OK;
}

esp_err_t sense_wire_event_source_read_wire(int index, int *out_value)
{
    int value = 0;
    ESP_RETURN_ON_ERROR(sense_wire_event_source_read_raw(index, &value), TAG, "Failed to read ADC value");

    if (value > sense_wires[index].threshold)
    {
        sense_wires[index].level = HIGH;
    }
    else
    {
        sense_wires[index].level = LOW;
    }

    return ESP_OK;
}
