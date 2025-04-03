#pragma once

#include <stdint.h>
#include "esp_err.h"

#define MPU6050_ADDRESS 0x68

// Bits de configuração do PWR_MGMT_1
#define MPU6050_PWR_MGMT_1_DEVICE_RESET 0x80 // Reseta todos os registradores
#define MPU6050_PWR_MGMT_1_SLEEP                                               \
    0x40 // Coloca o sensor em modo de baixo consumo
#define MPU6050_PWR_MGMT_1_CYCLE                                               \
    0x20 // Ativa o modo de ciclo (liga/desliga periodicamente)
#define MPU6050_PWR_MGMT_1_TEMP_DIS 0x08 // Desativa o sensor de temperatura
#define MPU6050_PWR_MGMT_1_CLKSEL_PLL_X                                        \
    0x01 // Usa o PLL do giroscópio no eixo X como clock

// Bits de configuração do PWR_MGMT_2
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_1_25HZ 0x00 // Wake-up a cada 1.25Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_5HZ 0x40    // Wake-up a cada 5Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_20HZ 0x80   // Wake-up a cada 20Hz
#define MPU6050_PWR_MGMT_2_LP_WAKE_CTRL_40HZ 0xC0   // Wake-up a cada 40Hz
#define MPU6050_PWR_MGMT_2_STBY_XA 0x20 // Desativa o acelerômetro no eixo X
#define MPU6050_PWR_MGMT_2_STBY_YA 0x10 // Desativa o acelerômetro no eixo Y
#define MPU6050_PWR_MGMT_2_STBY_ZA 0x08 // Desativa o acelerômetro no eixo Z
#define MPU6050_PWR_MGMT_2_STBY_XG 0x04 // Desativa o giroscópio no eixo X
#define MPU6050_PWR_MGMT_2_STBY_YG 0x02 // Desativa o giroscópio no eixo Y
#define MPU6050_PWR_MGMT_2_STBY_ZG 0x01 // Desativa o giroscópio no eixo Z

#define MPU6050_SIG_PATH_RESET_TEMP_RST 0x01  // Reseta o sensor de temperatura
#define MPU6050_SIG_PATH_RESET_ACCEL_RST 0x02 // Reseta o acelerômetro
#define MPU6050_SIG_PATH_RESET_GYRO_RST 0x04  // Reseta o giroscópio

typedef enum {
    MPU6050_CONFIG_AFS0 = 0, // ±2g
    MPU6050_CONFIG_AFS1 = 1, // ±4g
    MPU6050_CONFIG_AFS2 = 2, // ±8g
    MPU6050_CONFIG_AFS3 = 3, // ±16g
} mpu6050_accel_fs_t;

typedef enum {
    MPU6050_CONFIG_GFS0 = 0, // ±250°/s
    MPU6050_CONFIG_GFS1 = 1, // ±500°/s
    MPU6050_CONFIG_GFS2 = 2, // ±1000°/s
    MPU6050_CONFIG_GFS3 = 3, // ±2000°/s
} mpu6050_gyro_fs_t;

typedef enum {
    MPU6050_CONFIG_FILTER_260HZ = 0,
    MPU6050_CONFIG_FILTER_184HZ = 1,
    MPU6050_CONFIG_FILTER_94HZ = 2,
    MPU6050_CONFIG_FILTER_44HZ = 3,
    MPU6050_CONFIG_FILTER_21HZ = 4,
    MPU6050_CONFIG_FILTER_10HZ = 5,
    MPU6050_CONFIG_FILTER_5HZ = 6, 
} mpu6050_filter_config_t;

typedef struct {
    float x;
    float y;
    float z;
} accel_data_t;

typedef struct {
    float x;
    float y;
    float z;
} gyro_data_t;

typedef struct {
    uint8_t addr;
    mpu6050_filter_config_t filter_config;
    mpu6050_accel_fs_t accel_fs;
    mpu6050_gyro_fs_t gyro_fs;
    accel_data_t accel_offset;
    gyro_data_t gyro_offset;
    uint16_t sensor_temp;
} mpu6050_dev_t;

esp_err_t mpu6050_init(mpu6050_dev_t *dev);
esp_err_t mpu6050_set_filter(mpu6050_dev_t *dev, mpu6050_filter_config_t filter_config);
esp_err_t mpu6050_soft_reset(mpu6050_dev_t *dev);
esp_err_t mpu6050_hard_reset(mpu6050_dev_t *dev);
esp_err_t mpu6050_pwr_mgmt_1_mode(mpu6050_dev_t *dev, uint8_t sleep, uint8_t cycle, uint8_t temp_dis, uint8_t clk_sel);
esp_err_t mpu6050_pwr_mgmt_2_mode(mpu6050_dev_t *dev, uint8_t lp_wake_ctrl, uint8_t stby_zg, uint8_t stby_yg, uint8_t stby_xg, uint8_t stby_za, uint8_t stby_ya, uint8_t stby_xa);

esp_err_t mpu6050_accel_set_config(mpu6050_dev_t *dev, mpu6050_accel_fs_t accel_fs);
esp_err_t mpu6050_gyro_set_config(mpu6050_dev_t *dev, mpu6050_gyro_fs_t gyro_fs);

esp_err_t mpu6050_calibrate_accel(mpu6050_dev_t *dev, uint16_t samples);
esp_err_t mpu6050_calibrate_gyro(mpu6050_dev_t *dev, uint16_t samples);

esp_err_t mpu6050_accel_read_data(mpu6050_dev_t *dev, accel_data_t *data);
esp_err_t mpu6050_gyro_read_data(mpu6050_dev_t *dev, gyro_data_t *data);

esp_err_t mpu6050_read_temp(mpu6050_dev_t *dev, int16_t *temp);