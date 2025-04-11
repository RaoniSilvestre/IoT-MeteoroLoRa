#pragma once

#include <stdint.h>
#include "esp_err.h"

#define QMC5883L_ADDR_DEFAULT 0x0D

typedef enum {
    QMC5883L_CONFIG_RNG_2G = 0,
    QMC5883L_CONFIG_RNG_8G,
} qmc5883l_config_rng_t;

typedef enum {
    QMC5883L_CONFIG_ODR_10HZ = 0,
    QMC5883L_CONFIG_ODR_50HZ,
    QMC5883L_CONFIG_ODR_100HZ,
    QMC5883L_CONFIG_ODR_200HZ,
} qmc5883l_config_odr_t;

typedef enum {
    QMC5883L_CONFIG_OSR_512 = 0,
    QMC5883L_CONFIG_OSR_256,
    QMC5883L_CONFIG_OSR_128,
    QMC5883L_CONFIG_OSR_64,
} qmc5883l_config_osr_t;

typedef enum {
    QMC5883L_CONFIG_MODE_STANDBY = 0,
    QMC5883L_CONFIG_MODE_CONTINUOUS,
} qmc5883l_config_mode_t;

typedef enum {
    QMC5883L_CONFIG_INT_EN = 0,
    QMC5883L_CONFIG_INT_DIS,
} qmc5883l_config_int_t;

typedef struct {
    float x;
    float y;
    float z;
} magneto_data_t;

typedef struct {
    qmc5883l_config_rng_t rng_config;
    qmc5883l_config_odr_t odr_config;
    qmc5883l_config_osr_t osr_config;
    qmc5883l_config_mode_t mode_config;
    qmc5883l_config_int_t int_config;
} qmc5883l_config_t;

typedef struct {
    uint8_t addr;
    qmc5883l_config_t sensor_config;
    magneto_data_t offset, scale;
} qmc5883l_dev_t;

esp_err_t qmc5883l_init(qmc5883l_dev_t *dev, void (*DRDY_ISR)(void));
esp_err_t qmc5883l_set_config(qmc5883l_dev_t *dev, qmc5883l_config_t *config);
esp_err_t qmc5883l_set_offset(qmc5883l_dev_t *dev, magneto_data_t *offset);
esp_err_t qmc5883l_set_scale(qmc5883l_dev_t *dev, magneto_data_t *scale);
esp_err_t qmc5883l_set_mode(qmc5883l_dev_t *dev, qmc5883l_config_mode_t *mode);
esp_err_t qmc5883l_soft_reset(qmc5883l_dev_t *dev);
esp_err_t qmc5883l_calibrate(qmc5883l_dev_t *dev);
esp_err_t qmc5883l_read_data(qmc5883l_dev_t *dev, magneto_data_t *data);
esp_err_t qmc5883l_read_temp(float *temp);