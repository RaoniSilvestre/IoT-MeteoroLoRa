#include "payload/i2c_sensor.h"
#include <Arduino.h>
#include <Wire.h>

esp_err_t sensor_write_reg(uint8_t addr, uint8_t reg, uint8_t data) {
    Wire.beginTransmission(addr);
    if (!Wire.write(reg)) {
        Serial.print("[I²C] Falha ao escrever no registrador: ");
        Serial.println(reg, HEX);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    if (!Wire.write(data)) {
        Serial.print("[I²C] Falha ao escrever o dado: ");
        Serial.println(data, HEX);
        Wire.endTransmission();
        return ESP_FAIL;
    }
    Wire.endTransmission();
    return ESP_OK;
}

esp_err_t sensor_read_reg(uint8_t addr, uint8_t reg, uint8_t *data) {
    Wire.beginTransmission(addr);
    if (!Wire.write(reg)) {
        Serial.print("[I²C] Falha ao escrever no registrador: ");
        Serial.println(reg, HEX);
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
        Serial.print("[I²C] Falha ao escrever no registrador: ");
        Serial.println(reg, HEX);
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