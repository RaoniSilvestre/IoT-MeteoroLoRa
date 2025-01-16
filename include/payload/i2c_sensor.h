#pragma once 

#include <stdint.h>
#include "esp_err.h"

esp_err_t sensor_write_reg(uint8_t addr, uint8_t reg, uint8_t data);
esp_err_t sensor_read_reg(uint8_t addr, uint8_t reg, uint8_t *data);
esp_err_t sensor_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);