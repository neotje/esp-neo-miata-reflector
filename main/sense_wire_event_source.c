#include "sense_wire_event_source.h"

/*
PRIVATE
*/

#define CONTINUE_ON_ERROR(err, format, ...)     \
    if (err != ESP_OK)                          \
    {                                           \
        ESP_LOGE(TAG, format, ##__VA_ARGS__);   \
        continue;                               \
    }

static const char *TAG = "sense_wire_event_source";

sense_wire_t sense_wires[] = {
    {

#if CONFIG_IDF_TARGET_ESP32S3
        .io_num = CONFIG_BOARD_SENS1_PIN,
#elif CONFIG_IDF_TARGET_ESP32
        .io_num = 32,
#endif
        .adc_unit = ADC_UNIT_1,
        .adc_channel = 0,
        .threshold_key = "threshold1",
        .debounce_key = "debounce1",
        .threshold = 0,
        .debounce = 0,
        .on_event_id = MARKER_EVENT_SENSE1_ON,
        .off_event_id = MARKER_EVENT_SENSE1_OFF,
        .last_state = 0,
        .current_state = 0,
        .debounceStartTime = 0,
    },
    {
#if CONFIG_IDF_TARGET_ESP32S3
        .io_num = CONFIG_BOARD_SENS2_PIN,
#elif CONFIG_IDF_TARGET_ESP32
        .io_num = 33,
#endif
        .adc_unit = ADC_UNIT_1,
        .adc_channel = 0,
        .threshold_key = "threshold2",
        .debounce_key = "debounce2",
        .threshold = 0,
        .debounce = 0,
        .on_event_id = MARKER_EVENT_SENSE2_ON,
        .off_event_id = MARKER_EVENT_SENSE2_OFF,
        .last_state = 0,
        .current_state = 0,
        .debounceStartTime = 0,
    }};

const size_t sense_wires_count = sizeof(sense_wires) / sizeof(sense_wire_t);

adc_oneshot_unit_handle_t adc_handle;

esp_timer_handle_t sense_wire_event_source_timer_handle = NULL;

static void sense_wire_event_source_config_update_handler(void *args, esp_event_base_t base, int32_t id, void *event_data)
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

static void sense_wire_event_source_task(void* args)
{
    esp_err_t err = ESP_OK;

    for (size_t i = 0; i < sense_wires_count; i++)
    {
        int64_t on_time;
        int64_t off_time;

        sense_wire_t *sense_wire = &sense_wires[i];

        int current_level = 0;
        err = sense_wire_event_source_read_wire(i, &current_level);

        CONTINUE_ON_ERROR(err, "Failed to read sense wire %d", i);

        if (sense_wire->last_state != current_level)
        {
            sense_wire->debounceStartTime = esp_timer_get_time();
        }

        if (esp_timer_get_time() - sense_wire->debounceStartTime < sense_wire->debounce * 1000 && sense_wire->current_state != current_level)
        {
            sense_wire->current_state = current_level;

            if (sense_wire->current_state == HIGH)
            {
                ESP_LOGD(TAG, "Sense wire %d ON", i);

                on_time = esp_timer_get_time();
                mode_event_linker_post(sense_wire->on_event_id, &on_time, sizeof(time_t));
            } 
            else
            {
                ESP_LOGD(TAG, "Sense wire %d OFF", i);

                off_time = esp_timer_get_time();
                mode_event_linker_post(sense_wire->off_event_id, &off_time, sizeof(time_t));
            }
        }

        sense_wire->last_state = current_level;
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
    ESP_LOGI(TAG, "Initializing sense wire event source. Wire count: %d", sense_wires_count);

#if CONFIG_IDF_TARGET_ESP32S3
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
#elif CONFIG_IDF_TARGET_ESP32
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
    };
#endif

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

        esp_err_t err = ESP_FAIL;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = init_config.unit_id,
            .chan = sense_wires[i].adc_channel,
            .atten = channel_config.atten,
            .bitwidth = channel_config.bitwidth,
        };

        err = adc_cali_create_scheme_curve_fitting(&cali_config, &sense_wires[i].cali_handle);
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = init_config.unit_id,
            .atten = channel_config.atten,
            .bitwidth = channel_config.bitwidth, 
            .default_vref = 1100,       
        };

        err = adc_cali_create_scheme_line_fitting(&cali_config, &sense_wires[i].cali_handle);
#endif

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

        ESP_LOGI(TAG, "Sense wire %d (threshold: %li, channel: %i)", i, sense_wires[i].threshold, sense_wires[i].adc_channel);
    }

    ESP_RETURN_ON_ERROR(config_manager_register_update_handler(sense_wire_event_source_config_update_handler, NULL), TAG, "Failed to add config update handler");

    esp_timer_create_args_t button_timer = {
        .arg = NULL,
        .callback = sense_wire_event_source_task,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "sense_wire_event_source_timer"
    };
    
    ESP_RETURN_ON_ERROR(esp_timer_create(&button_timer, &sense_wire_event_source_timer_handle), TAG, "Failed to create sense wire event source timer");

    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(sense_wire_event_source_timer_handle, CONFIG_SENSE_WIRE_EVENT_SOURCE_TIMER_INTERVAL_MS * 1000), TAG, "Failed to start sense wire event source timer");

    return ESP_OK;
}

esp_err_t sense_wire_event_source_read_wire(int index, int *out_value)
{
    int value = 0;
    ESP_RETURN_ON_ERROR(sense_wire_event_source_read_raw(index, &value), TAG, "Failed to read ADC value");

    //ESP_LOGI(TAG, "Sense wire %d raw value: %d", index, value);

    if (value > sense_wires[index].threshold)
    {
        value = HIGH;
    }
    else
    {
        value = LOW;
    }

    *out_value = value;

    return ESP_OK;
}

int sense_wire_event_source_get_state(int index)
{
    if (index < 0 || index >= sense_wires_count)
    {
        ESP_LOGE(TAG, "Invalid sense wire index %d", index);
        return -1;
    }

    return sense_wires[index].current_state;
}
