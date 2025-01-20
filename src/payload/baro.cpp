#include "payload/baro.h"
#include "payload/i2c_sensor.h"
#include <Arduino.h>
#include <math.h>

// Registradores do BMP180
#define BMP180_REG_CALIB_START   0xAA  // Início dos coeficientes de calibração
#define BMP180_REG_CONTROL       0xF4  // Registrador de controle
#define BMP180_REG_RESULT        0xF6  // Registrador de saída (MSB)
#define BMP180_REG_SOFT_RESET    0xE0  // Reset do sensor
#define BMP180_REG_ID            0xD0  // ID do chip

baro_calib_data_t cal_data;

// Inicialização do BMP180
esp_err_t baro_init() {
    uint8_t chip_id;
    if (sensor_read_reg(BMP180_ADDR, BMP180_REG_ID, &chip_id) != ESP_OK || chip_id != 0x55) {
        Serial.println("[Baro] Falha ao identificar o sensor");
        return ESP_FAIL;
    }
    if (baro_set_oversampling(BMP180_OSS_STANDARD) != ESP_OK) {
        return ESP_FAIL;
    }
    return baro_read_calibration(&cal_data);
}

// Reset do BMP180
esp_err_t baro_soft_reset() {
    Serial.println("[Baro] Resetando o sensor");
    return sensor_write_reg(BMP180_ADDR, BMP180_REG_SOFT_RESET, 0xB6);
}

// Leitura dos coeficientes de calibração
esp_err_t baro_read_calibration(baro_calib_data_t *calib_data) {
    uint8_t buffer[22];
    if (sensor_read_reg(BMP180_ADDR, BMP180_REG_CALIB_START, buffer, 22) != ESP_OK) {
        Serial.println("[Baro] Falha ao ler os coeficientes de calibração");
        return ESP_FAIL;
    }
    calib_data->ac1 = (buffer[0] << 8) | buffer[1];
    calib_data->ac2 = (buffer[2] << 8) | buffer[3];
    calib_data->ac3 = (buffer[4] << 8) | buffer[5];
    calib_data->ac4 = (buffer[6] << 8) | buffer[7];
    calib_data->ac5 = (buffer[8] << 8) | buffer[9];
    calib_data->ac6 = (buffer[10] << 8) | buffer[11];
    calib_data->b1 = (buffer[12] << 8) | buffer[13];
    calib_data->b2 = (buffer[14] << 8) | buffer[15];
    calib_data->mb = (buffer[16] << 8) | buffer[17];
    calib_data->mc = (buffer[18] << 8) | buffer[19];
    calib_data->md = (buffer[20] << 8) | buffer[21];
    return ESP_OK;
}

// Configuração do Oversampling
esp_err_t baro_set_oversampling(uint8_t oss) {
    Serial.printf("[Baro] Configurando oversampling: 0x%02X\n", oss);
    cal_data.oss = oss;
    return ESP_OK;
}

// Cálculo da altitude
esp_err_t baro_read_altitude(float *altitude, float sea_level_pressure) {
    float pressure;
    *altitude = 44330.0 * (1.0 - pow(pressure / sea_level_pressure, 0.1903));
    return ESP_OK;
}

int32_t compute_b5(int32_t ut) {
    int32_t x1 = ((ut - cal_data.ac6) * cal_data.ac5) >> 15;
    int32_t x2 = (cal_data.mc << 11) / (x1 + cal_data.md);
    return x1 + x2;
}

// Leitura completa dos dados do sensor
esp_err_t baro_read_data(baro_data_t *data) {
    uint8_t buffer[2];
    if (sensor_write_reg(BMP180_ADDR, BMP180_REG_CONTROL, BMP180_CMD_MEASURE_TEMP) != ESP_OK) {
        Serial.println("[Baro] Falha ao enviar comando de leitura da temperatura");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
    if (sensor_read_reg(BMP180_ADDR, BMP180_REG_RESULT, buffer, 2) != ESP_OK) {
        return ESP_FAIL;
    }
    int16_t ut = (buffer[0] << 8) | buffer[1];
    int32_t b5 = compute_b5(ut);
    data->temperature = ((b5 + 8) >> 4) / 10.0;

    uint8_t buffer_pres[3];
    if (sensor_write_reg(BMP180_ADDR, BMP180_REG_CONTROL, BMP180_CMD_MEASURE_PRES | (cal_data.oss << 8)) != ESP_OK) {
        Serial.println("[Baro] Falha ao enviar comando de leitura da pressão");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(8 * cal_data.oss));
    if (sensor_read_reg(BMP180_ADDR, BMP180_REG_RESULT, buffer, 3) != ESP_OK) {
        Serial.println("[Baro] Falha ao ler os registradores de Pressão");
        return ESP_FAIL;
    }
    int32_t up = ((buffer_pres[0] << 16) | (buffer_pres[1] << 8) | buffer_pres[2]) >> 5;
    int32_t b6 = b5 - 4000;
    int32_t x1 = ((int32_t)cal_data.b2 * (b6 * b6 >> 12)) >> 11;
    int32_t x2 = ((int32_t)cal_data.ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = ((((int32_t)cal_data.ac1 * 4 + x3) << cal_data.oss) + 2) / 4;
    x1 = ((int32_t)cal_data.ac3 * b6) >> 13;
    x2 = ((int32_t)cal_data.b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = ((int32_t)cal_data.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * (int32_t)(50000UL >> cal_data.oss);
    int32_t p = (b7 < 0x80000000) ? (b7 * 2) / b4 : (b7 / b4) * 2;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + (int32_t)3791) >> 4);

    data->pressure = p / 100.0;
    return ESP_OK;
}
