#pragma once

#include <stdint.h>
#include "esp_err.h"

#define QMC5883L_ADDR 0x0D

#define QMC5883L_CTRL_1_MODE_STANDBY 0x00
#define QMC5883L_CTRL_1_MODE_CONTINUOUS 0x01

#define QMC5883L_CTRL_1_ODR_10HZ 0x00
#define QMC5883L_CTRL_1_ODR_50HZ 0x04
#define QMC5883L_CTRL_1_ODR_100HZ 0x08
#define QMC5883L_CTRL_1_ODR_200HZ 0x0C

#define QMC5883L_CTRL_1_RNG_2G 0x00
#define QMC5883L_CTRL_1_RNG_8G 0x10

#define QMC5883L_CTRL_1_OSR_512 0x00
#define QMC5883L_CTRL_1_OSR_256 0x20
#define QMC5883L_CTRL_1_OSR_128 0x40
#define QMC5883L_CTRL_1_OSR_64 0x60

#define QMC5883L_CTRL_2_INT_DIS 0x00
#define QMC5883L_CTRL_2_INT_ENA 0x01
#define QMC5883L_CTRL_2_ROL_PNT 0x40
#define QMC5883L_CTRL_2_SFT_RST 0x80

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} magneto_data_t;

esp_err_t magneto_init();
esp_err_t magneto_set_mode(uint8_t mode, uint8_t odr, uint8_t rng, uint8_t osr);
esp_err_t magneto_soft_reset();
esp_err_t magneto_calibrate();
esp_err_t magneto_read_status(uint8_t *status);
esp_err_t magneto_read_data(magneto_data_t *data);
esp_err_t magneto_read_temp(int16_t *temp);
