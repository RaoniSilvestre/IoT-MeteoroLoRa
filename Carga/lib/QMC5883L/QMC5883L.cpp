#include <Arduino.h>
#include "I2C_sensor.h"
#include "QMC5883L.h"

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

float _offset[] = {0, 0, 0};
float _scale[] = {1, 1, 1};

esp_err_t qmc5883l_init() {
    if (sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_SET_RESET_PERIOD, 0x01) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o período de reset");
        return ESP_FAIL;
    }
    if (qmc5883l_set_mode(QMC5883L_CTRL_1_MODE_CONTINUOUS, QMC5883L_CTRL_1_ODR_200HZ, QMC5883L_CTRL_1_RNG_8G, QMC5883L_CTRL_1_OSR_512) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o registrador de controle do sensor");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    if (qmc5883l_calibrate() != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha na calibração do sensor, utilizando valores padrão");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t qmc5883l_set_mode(uint8_t mode, uint8_t odr, uint8_t rng, uint8_t osr) {
    uint8_t ctrl1 = mode | odr | rng | osr;
    Serial.printf("[QMC5883L (Magneto)] Configurando modo: 0x%02X\n", ctrl1);
    return sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_CTRL_1, ctrl1);
}

esp_err_t qmc5883l_soft_reset() {
    Serial.println("[qmc5883l] Resetando o sensor");
    return sensor_write_reg(QMC5883L_ADDR, QMC5883L_REG_CTRL_2, QMC5883L_CTRL_2_SFT_RST);
}

esp_err_t qmc5883l_read_status(uint8_t *status) {
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_STATUS, status) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler status");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t qmc5883l_calibrate() {
    Serial.println("[QMC5883L (Magneto)] Calibrando o sensor");
    long cal_data[3][2] = {{65000, -65000}, {65000, -65000}, {65000, -65000}};
    magneto_data_t data;

  	if(qmc5883l_read_data(&data) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha na leitura do sensor para calibração");
        return ESP_FAIL;
    }

    long x = cal_data[0][0] = cal_data[0][1] = data.x;
    long y = cal_data[1][0] = cal_data[1][1] = data.y;
  	long z = cal_data[2][0] = cal_data[2][1] = data.z;

	unsigned long start_time = millis();

	while((millis() - start_time) < 10000) {
        vTaskDelay(pdMS_TO_TICKS(100));
		if(qmc5883l_read_data(&data) != ESP_OK) {
            Serial.println("[QMC5883L (Magneto)] Falha na leitura do sensor para calibração");
            _offset[0] = _offset[1] = _offset[2] = 0;
            _scale[0] = _scale[1] = _scale[2] = 1;
            return ESP_FAIL;
        }

  		x = data.x;
  		y = data.y;
  		z = data.z;

		if(x < cal_data[0][0]) {
			cal_data[0][0] = x;
		}
		if(x > cal_data[0][1]) {
			cal_data[0][1] = x;
		}

		if(y < cal_data[1][0]) {
			cal_data[1][0] = y;
		}
		if(y > cal_data[1][1]) {
			cal_data[1][1] = y;
		}

		if(z < cal_data[2][0]) {
			cal_data[2][0] = z;
		}
		if(z > cal_data[2][1]) {
			cal_data[2][1] = z;
		}
	}

    _offset[0] = (cal_data[0][0] + cal_data[0][1]) / 2;
    _offset[1] = (cal_data[1][0] + cal_data[1][1]) / 2;
    _offset[2] = (cal_data[2][0] + cal_data[2][1]) / 2;

    float x_avg_delta = (cal_data[0][0] - cal_data[0][1]) / 2;
    float y_avg_delta = (cal_data[1][0] - cal_data[1][1]) / 2;
    float z_avg_delta = (cal_data[2][0] - cal_data[2][1]) / 2;

    float avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3;

    _scale[0] = avg_delta / x_avg_delta;
    _scale[1] = avg_delta / y_avg_delta;
    _scale[2] = avg_delta / z_avg_delta;
    Serial.println("[QMC5883L (Magneto)] Sensor calibrado");
    return ESP_OK;
}

esp_err_t qmc5883l_read_data(magneto_data_t *data) {
    uint8_t status;
    if (qmc5883l_read_status(&status) != ESP_OK) {
        return ESP_FAIL;
    }
    if (!(status & 0x01)) {
        Serial.println("[QMC5883L (Magneto)] Dados não prontos");
        return ESP_FAIL;
    }
    uint8_t buffer[6];
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_X_LSB, buffer, 6) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler dados");
        return ESP_FAIL;
    }
    data->x = (int16_t)(buffer[1] << 8 | buffer[0]);
    data->y = (int16_t)(buffer[3] << 8 | buffer[2]);
    data->z = (int16_t)(buffer[5] << 8 | buffer[4]);

    data->x = (data->x - _offset[0]) * _scale[0];
    data->y = (data->y - _offset[1]) * _scale[1];
    data->z = (data->z - _offset[2]) * _scale[2];

    return ESP_OK;
}

esp_err_t qmc5883l_read_temp(float *temp) {
    uint8_t buffer[2];
    if (sensor_read_reg(QMC5883L_ADDR, QMC5883L_REG_TEMP_LSB, buffer, 2) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler temperatura");
        return ESP_FAIL;
    }
    *temp = ((int16_t)(buffer[1] << 8 | buffer[0])) / 100;
    return ESP_OK;
}