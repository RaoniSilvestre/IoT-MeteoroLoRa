#include "payload/magneto.h"
#include "payload/i2c_sensor.h"
#include <Arduino.h>

#define QMC5883L_REG_X_LSB 0x00
#define QMC5883L_REG_X_MSB 0x01
#define QMC5883L_REG_Y_LSB 0x02
#define QMC5883L_REG_Y_MSB 0x03
#define QMC5883L_REG_Z_LSB 0x04
#define QMC5883L_REG_Z_MSB 0x05
#define QMC5883L_REG_STATUS 0x06
#define QMC5883L_REG_TEMP_LSB 0x07
#define QMC5883L_REG_TEMP_MSB 0x08
#define QMC5883L_REG_CTRL_1 0x09
#define QMC5883L_REG_CTRL_2 0x0A
#define QMC5883L_REG_SET_RESET_PERIOD 0x0B

esp_err_t magneto_init() {
    if (sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_SET_RESET_PERIOD, 0x01) != ESP_OK) {
        Serial.println("[Magneto] Falha ao configurar o período de reset");
        return ESP_FAIL;
    }
    return magneto_set_mode(QMC5883L_CTRL_1_MODE_CONTINUOUS, QMC5883L_CTRL_1_ODR_200HZ, QMC5883L_CTRL_1_RNG_8G, QMC5883L_CTRL_1_OSR_512);
}

esp_err_t magneto_set_mode(uint8_t mode, uint8_t odr, uint8_t rng, uint8_t osr) {
    uint8_t ctrl1 = mode | odr | rng | osr;
    Serial.printf("[Magneto] Configurando modo: 0x%02X\n", ctrl1);
    return sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_CTRL_1, ctrl1);
}

esp_err_t magneto_soft_reset() {
    Serial.println("[Magneto] Resetando o sensor");
    return sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_CTRL_2, QMC5883L_CTRL_2_SFT_RST);
}

esp_err_t magneto_read_status(uint8_t *status) {
    uint8_t *magneto_status;
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_STATUS, magneto_status) != ESP_OK) {
        Serial.println("[Magneto] Falha ao ler status");
        return ESP_FAIL;
    }
    status = magneto_status;
}

esp_err_t magneto_read_data(magneto_data_t *data) {
    uint8_t *status;
    magneto_read_status(status);
    if (!(*status & 0x01)) {
        Serial.println("[Magneto] Dados não prontos");
        return ESP_FAIL;
    }
    uint8_t buffer[6];
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_X_LSB, buffer, 6) != ESP_OK) {
        Serial.println("[Magneto] Falha ao ler dados");
        return ESP_FAIL;
    }
    data->x = (int16_t)(buffer[1] << 8 | buffer[0]);
    data->y = (int16_t)(buffer[3] << 8 | buffer[2]);
    data->z = (int16_t)(buffer[5] << 8 | buffer[4]);
    return ESP_OK;
}

esp_err_t magneto_read_temp(int16_t *temp) {
    uint8_t buffer[2];
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_TEMP_LSB, buffer, 2) != ESP_OK) {
        Serial.println("[Magneto] Falha ao ler temperatura");
        return ESP_FAIL;
    }
    *temp = (int16_t)(buffer[1] << 8 | buffer[0]);
    return ESP_OK;
}