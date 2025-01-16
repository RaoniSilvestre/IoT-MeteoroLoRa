#include "payload/termo.h"
#include "payload/i2c_sensor.h"
#include <Arduino.h>

#define AHT10_CMD_CALIBRATE 0xE1
#define AHT10_CMD_RESET 0xBA
#define AHT10_CMD_TRIG_MEASURE 0xAC

//#define AHT10_STATUS_CALIBRATED 0x08
#define AHT10_RESET_TIME_MS 20
#define AHT10_MEASURE_TIME_MS 80

esp_err_t termo_init() {
    esp_err_t ret = termo_calibrate();
    return ret;
}

esp_err_t termo_calibrate() {
    Serial.println("[Termo] Calibrando o sensor");
    esp_err_t ret = sensor_write_cmd(AHT10_ADDR, AHT10_CMD_CALIBRATE);
    vTaskDelay(pdMS_TO_TICKS(AHT10_RESET_TIME_MS));
    return ret;
}

esp_err_t termo_soft_reset() {
    Serial.println("[Termo] Resetando o sensor");
    esp_err_t ret = sensor_write_cmd(AHT10_ADDR, AHT10_CMD_RESET);
    vTaskDelay(pdMS_TO_TICKS(AHT10_RESET_TIME_MS));
    return ret;
}

esp_err_t termo_read_status(uint8_t *status) {
    return sensor_read_cmd(AHT10_ADDR, status);
}

esp_err_t termo_read_data(termo_data_t *data) {
    if (sensor_write_reg(AHT10_ADDR, AHT10_CMD_TRIG_MEASURE, 0x00) != ESP_OK ) {
        Serial.println("[Termo] Falha ao enviar comando de medição");
        return ESP_FAIL;
    }
    uint8_t status;
    if (termo_read_status(&status) != ESP_OK) {
        Serial.println("[Termo] Falha ao coletar status do sensor");
        return ESP_FAIL;
    }
    if (!(status & AHT10_STATUS_CALIBRATED)) {
        if (termo_calibrate() != ESP_OK) {
            Serial.println("[Termo] Falha ao calibrar o sensor");
            return ESP_FAIL;
        }
    } else if (status & AHT10_STATUS_BUSY) {
        Serial.println("[Termo] Sensor ocupado, aguardando");
        vTaskDelay(pdMS_TO_TICKS(AHT10_MEASURE_TIME_MS));
    }
    
    uint8_t buffer[6];
    if (sensor_read_cmd(AHT10_ADDR, buffer, 6) != ESP_OK) {
        Serial.println("[Termo] Falha ao ler dados do sensor");
        return ESP_FAIL;
    }

    uint32_t raw = ((uint32_t)buffer[1] << 12) | ((uint32_t)buffer[2] << 4) | ((uint32_t)buffer[3] >> 4);

    data->humd = (float)raw * 100 / (1<<20);

    raw = ((uint32_t)(buffer[3] & 0x0F) << 16) | ((uint32_t)buffer[4] << 8) | buffer[5];

    data->temp = (float)raw * 200 / (1<<20) - 50;
    
    return ESP_OK;
}