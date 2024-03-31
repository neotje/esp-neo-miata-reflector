#ifndef SENSE_SOURCE_H
#define SENSE_SOURCE_H

#include <stdint.h>
#include "esp_check.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

typedef struct {


} sense_source_pin_t;

esp_err_t sense_source_init();

esp_err_t sense_source_deinit();

#endif