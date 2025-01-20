#pragma once

#include <stdint.h>
#include "esp_err.h"

#define MPU6050_ADDRESS 0x68

#define MPU6050_GYRO_CONFIG_FS_250 0x00  // ±250°/s
#define MPU6050_GYRO_CONFIG_FS_500 0x08  // ±500°/s
#define MPU6050_GYRO_CONFIG_FS_1000 0x10 // ±1000°/s
#define MPU6050_GYRO_CONFIG_FS_2000 0x18 // ±2000°/s

#define MPU6050_ACCEL_CONFIG_FS_2G 0x00  // ±2g
#define MPU6050_ACCEL_CONFIG_FS_4G 0x08  // ±4g
#define MPU6050_ACCEL_CONFIG_FS_8G 0x10  // ±8g
#define MPU6050_ACCEL_CONFIG_FS_16G 0x18 // ±16g

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

#define MPU6050_FILTER_CONFIG_260HZ 0x00 // Filtro de 260Hz
#define MPU6050_FILTER_CONFIG_184HZ 0x01 // Filtro de 184Hz
#define MPU6050_FILTER_CONFIG_94HZ 0x02  // Filtro de 94Hz
#define MPU6050_FILTER_CONFIG_44HZ 0x03  // Filtro de 44Hz
#define MPU6050_FILTER_CONFIG_21HZ 0x04  // Filtro de 21Hz
#define MPU6050_FILTER_CONFIG_10HZ 0x05  // Filtro de 10Hz
#define MPU6050_FILTER_CONFIG_5HZ 0x06   // Filtro de 5Hz

#define MPU6050_SIG_PATH_RESET_TEMP_RST 0x01  // Reseta o sensor de temperatura
#define MPU6050_SIG_PATH_RESET_ACCEL_RST 0x02 // Reseta o acelerômetro
#define MPU6050_SIG_PATH_RESET_GYRO_RST 0x04  // Reseta o giroscópio

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

esp_err_t accel_gyro_init();
esp_err_t accel_gyro_set_filter(uint8_t filter_config);
esp_err_t accel_gyro_soft_reset();
esp_err_t accel_gyro_hard_reset();
esp_err_t accel_gyro_pwr_mgmt_1_mode(uint8_t sleep, uint8_t cycle, uint8_t temp_dis, uint8_t clk_sel);
esp_err_t accel_gyro_pwr_mgmt_2_mode(uint8_t lp_wake_ctrl, uint8_t stby_zg, uint8_t stby_yg, uint8_t stby_xg, uint8_t stby_za, uint8_t stby_ya, uint8_t stby_xa);

esp_err_t accel_set_config(uint8_t accel_fs);
esp_err_t gyro_set_config(uint8_t gyro_fs);

esp_err_t accel_set_offset(int16_t x, int16_t y, int16_t z);
esp_err_t gyro_set_offset(int16_t x, int16_t y, int16_t z);

esp_err_t accel_read_data(accel_data_t *data);
esp_err_t gyro_read_data(gyro_data_t *data);

esp_err_t accel_gyro_read_temp(int16_t *temp);
