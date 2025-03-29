#pragma once

#include <stdint.h>
#include "esp_err.h"

#define BMP180_ADDR 0x77  // Endereço I2C do BMP180

// Comandos para medições
#define BMP180_CMD_MEASURE_TEMP  0x2E  // Iniciar medição de temperatura
#define BMP180_CMD_MEASURE_PRES  0x34  // Iniciar medição de pressão (modo padrão)

// Modos de Oversampling para Pressão
#define BMP180_OSS_ULTRA_LOW     0  // Ultra Low Power
#define BMP180_OSS_STANDARD      1  // Standard
#define BMP180_OSS_HIGH          2  // High Resolution
#define BMP180_OSS_ULTRA_HIGH    3  // Ultra High Resolution

// Estruturas de dados
typedef struct {
    int16_t ac1, ac2, ac3;
    uint16_t ac4, ac5, ac6;
    int16_t b1, b2;
    int16_t mb, mc, md;
    uint8_t oss;
} baro_calib_data_t;

typedef struct {
    float temperature;
    float pressure;
    float altitude;
} baro_data_t;

esp_err_t baro_init();
esp_err_t baro_soft_reset();
esp_err_t baro_read_calibration(baro_calib_data_t *calib_data);
esp_err_t baro_set_oversampling(uint8_t oss);
esp_err_t baro_read_altitude(float *altitude, float sea_level_pressure);
esp_err_t baro_read_data(baro_data_t *data);