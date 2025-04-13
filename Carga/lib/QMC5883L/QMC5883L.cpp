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
#define QMC5883L_REG_WHO_AM_I 0x0D

#define QMC5883L_CTRL_2_ROL_PNT 0x40
#define QMC5883L_CTRL_2_SFT_RST 0x80

float _offset[] = {0, 0, 0};
float _scale[] = {1, 1, 1};

esp_err_t qmc5883l_init(qmc5883l_dev_t *dev, void (*DRDY_ISR)(void)) {
    if (dev == NULL) {
        Serial.println("[QMC5883L (Magneto)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->addr == 0) {
        dev->addr = QMC5883L_ADDR_DEFAULT;
    }

    uint8_t who_am_i;

    if (sensor_read_reg(dev->addr, QMC5883L_REG_WHO_AM_I, &who_am_i, 1) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler o registrador WHO_AM_I");
        return ESP_FAIL;
    }

    if (sensor_write_reg(dev->addr, QMC5883L_REG_SET_RESET_PERIOD, 0x01) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o período de reset");
        return ESP_FAIL;
    }

    if (qmc5883l_set_config(dev, &dev->sensor_config) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o sensor");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t qmc5883l_set_config(qmc5883l_dev_t *dev, qmc5883l_config_t *config) {
    if (dev == NULL || config == NULL) {
        Serial.println("[QMC5883L (Magneto)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t ctrl1 = config->mode_config | config->odr_config | config->rng_config | config->osr_config;
    uint8_t ctrl2 = config->int_config;

    if (sensor_write_reg(dev->addr, QMC5883L_REG_CTRL_1, ctrl1) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o registrador CTRL_1");
        return ESP_FAIL;
    }

    if (sensor_write_reg(dev->addr, QMC5883L_REG_CTRL_2, ctrl2) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao configurar o registrador CTRL_2");
        return ESP_FAIL;
    }

    dev->sensor_config = *config;

    return ESP_OK;
}

esp_err_t qmc5883l_soft_reset(qmc5883l_dev_t *dev) {
    if (dev == NULL) {
        Serial.println("[QMC5883L (Magneto)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    Serial.println("[QMC5883L (Magneto] Resetando o sensor");
    return sensor_write_reg(dev->addr, QMC5883L_REG_CTRL_2, QMC5883L_CTRL_2_SFT_RST);
}

esp_err_t qmc5883l_read_status(qmc5883l_dev_t *dev) {
    if (dev == NULL) {
        Serial.println("[QMC5883L (Magneto)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status_reg;
    if (sensor_read_reg(dev->addr, QMC5883L_REG_STATUS, &status_reg, 1) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler o registrador de status");
        return ESP_FAIL;
    }
    
    dev->status.data_ready = (status_reg & 0x01) ? 1 : 0;
    dev->status.overflow = (status_reg & 0x02) ? 1 : 0;
    dev->status.data_skip = (status_reg & 0x04) ? 1 : 0;

    return ESP_OK;
}

esp_err_t qmc5883l_calibrate(qmc5883l_dev_t *dev) {
    Serial.println("[QMC5883L (Magneto)] Calibrando o sensor");
    long cal_data[3][2] = {{65000, -65000}, {65000, -65000}, {65000, -65000}};
    magneto_data_t data;

  	if(qmc5883l_read_data(dev, &data) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha na leitura do sensor para calibração");
        return ESP_FAIL;
    }

    long x = cal_data[0][0] = cal_data[0][1] = data.x;
    long y = cal_data[1][0] = cal_data[1][1] = data.y;
  	long z = cal_data[2][0] = cal_data[2][1] = data.z;

	unsigned long start_time = millis();

	while((millis() - start_time) < 10000) {
        vTaskDelay(pdMS_TO_TICKS(100));
		if(qmc5883l_read_data(dev, &data) != ESP_OK) {
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

esp_err_t qmc5883l_read_data(qmc5883l_dev_t *dev, magneto_data_t *data) {
    if (dev == NULL) {
        Serial.println("[QMC5883L (Magneto)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[6];
    if (sensor_read_reg(dev->addr, QMC5883L_REG_X_LSB, buffer, 6) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler dados");
        return ESP_FAIL;
    }

    uint16_t scale = dev->sensor_config.rng_config == QMC5883L_CONFIG_RNG_2G ? 12000 : 3000;

    data->x = ((int16_t)(buffer[1] << 8 | buffer[0])) / scale;
    data->y = ((int16_t)(buffer[3] << 8 | buffer[2])) / scale;
    data->z = ((int16_t)(buffer[5] << 8 | buffer[4])) / scale;

    return ESP_OK;
}

esp_err_t qmc5883l_read_temp(qmc5883l_dev_t *dev, float *temp) {
    uint8_t buffer[2];
    if (sensor_read_reg(dev->addr, QMC5883L_REG_TEMP_LSB, buffer, 2) != ESP_OK) {
        Serial.println("[QMC5883L (Magneto)] Falha ao ler temperatura");
        return ESP_FAIL;
    }
    *temp = ((int16_t)(buffer[1] << 8 | buffer[0])) / 100;
    return ESP_OK;
}