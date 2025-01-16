#include "payload/i2c_sensor.h"
#include <Arduino.h>
#include <Wire.h>

esp_err_t sensor_write_cmd(uint8_t addr, uint8_t cmd) {
    Wire.beginTransmission(addr);
    if (!Wire.write(cmd)) {
        Serial.printf("[I²C] Falha ao enviar o comando: 0x%02X para o dispositivo: 0x%02X\n", cmd, addr);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    Wire.endTransmission();
    return ESP_OK;
}

esp_err_t sensor_write_reg(uint8_t addr, uint8_t reg, uint8_t data) {
    Wire.beginTransmission(addr);
    if (!Wire.write(reg)) {
        Serial.printf("[I²C] Falha ao escrever no registrador: 0x%02X do dispositivo: 0x%02X\n", reg, addr);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    if (!Wire.write(data)) {
        Serial.printf("[I²C] Falha ao escrever o dado: 0x%02X no registrador 0x%02X do dispositivo 0x%02X\n", data, reg, addr);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    Wire.endTransmission();
    return ESP_OK;
}

esp_err_t sensor_read_cmd(uint8_t addr, uint8_t *cmd) {
    Wire.beginTransmission(addr);
    Wire.requestFrom(addr, 1);
    if (!Wire.available()) {
        Serial.println("[I²C] Nenhum dado disponível");
        return ESP_FAIL;
    }
    *cmd = Wire.read();
    return ESP_OK;
}

esp_err_t sensor_read_cmd(uint8_t addr, uint8_t *cmd, size_t len) {
    Wire.beginTransmission(addr);
    Wire.requestFrom(addr, len);
    if (Wire.available() < len) {
        Serial.println("[I²C] Dados insuficientes");
        return ESP_FAIL;
    }
    for (size_t i = 0; i < len; i++) {
        cmd[i] = Wire.read();
    }
    return ESP_OK;
}
esp_err_t sensor_read_reg(uint8_t addr, uint8_t reg, uint8_t *data) {
    Wire.beginTransmission(addr);
    if (!Wire.write(reg)) {
        Serial.printf("[I²C] Falha ao ler do registrador: 0x%02X do dispositivo: 0x%02X\n", reg, addr);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    if (Wire.endTransmission() != 0) {
        Serial.println("[I²C] Falha ao finalizar transmissão");
        return ESP_FAIL;
    }
    Wire.requestFrom(addr, 1);
    if (!Wire.available()) {
        Serial.println("[I²C] Nenhum dado disponível");
        return ESP_FAIL;
    }
    *data = Wire.read();
    return ESP_OK;
}

esp_err_t sensor_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    Wire.beginTransmission(addr);
    if (!Wire.write(reg)) {
        Serial.printf("[I²C] Falha ao ler do registrador: 0x%02X do dispositivo: 0x%02X\n", reg, addr);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    if (Wire.endTransmission() != 0) {
        Serial.println("[I²C] Falha ao finalizar transmissão");
        return ESP_FAIL;
    }
    Wire.requestFrom(addr, len);
    if (Wire.available() < len) {
        Serial.println("[I²C] Dados insuficientes");
        return ESP_FAIL;
    }
    for (size_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }
    return ESP_OK;
}