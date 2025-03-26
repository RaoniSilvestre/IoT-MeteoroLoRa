#pragma once

#include <stdint.h>
#include "esp_err.h"

#define AHT10_ADDR 0x38

#define AHT10_STATUS_CALIBRATED 0x68
#define AHT10_STATUS_BUSY 0x80

typedef struct {
    float temp;
    float humd;
} termo_data_t;

esp_err_t termo_init();
esp_err_t termo_calibrate();
esp_err_t termo_soft_reset();
esp_err_t termo_read_status(uint8_t *status);
esp_err_t termo_read_data(termo_data_t *data);