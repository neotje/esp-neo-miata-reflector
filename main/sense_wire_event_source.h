#ifndef SENSOR_WIRE_EVENT_SOURCE_H
#define SENSOR_WIRE_EVENT_SOURCE_H

#include <stdint.h>
#include "time.h"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_timer.h"

#include "mode_event_linker.h"
#include "config_manager.h"

#define HIGH 1
#define LOW 0

#define TIME_TO_TICKS(time) (time / CONFIG_SENSE_WIRE_EVENT_SOURCE_TIMER_INTERVAL_MS)

typedef struct {
    int io_num;

    adc_unit_t adc_unit;
    adc_channel_t adc_channel;
    adc_cali_handle_t cali_handle;

    const char* threshold_key;
    const char* debounce_key;
    int32_t threshold;
    int32_t debounce; // in ms

    int32_t on_event_id;
    int32_t off_event_id;

    int level;
    int debounce_counter; // in ticks of SENSE_WIRE_EVENT_SOURCE_TIMER_INTERVAL_MS
} sense_wire_t;

/**
 * @brief Initialize the sense wire event source: Configure the sense wires and the ADC, subscribe to the config manager update event, and event source task.
 * 
 * @return esp_err_t 
 */
esp_err_t sense_wire_event_source_init();

esp_err_t sense_wire_event_source_read_wire(int index, int* out_value);

esp_err_t sense_wire_event_source_get(int index, sense_wire_t** out_sense_wire);

#endif // SENSOR_WIRE_EVENT_SOURCE_H