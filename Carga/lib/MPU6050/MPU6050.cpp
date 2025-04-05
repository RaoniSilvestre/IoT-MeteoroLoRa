#include <Arduino.h>
#include "I2C_sensor.h"
#include "MPU6050.h"

// Registradores principais
#define MPU6050_REG_SELF_TEST_X 0x0D
#define MPU6050_REG_SELF_TEST_Y 0x0E
#define MPU6050_REG_SELF_TEST_Z 0x0F
#define MPU6050_REG_SELF_TEST_A 0x10

#define MPU6050_REG_SMPLRT_DIV 0x19    // Divisor de taxa de amostragem
#define MPU6050_REG_FILTER_CONFIG 0x1A // Configuração do filtro passa-baixa
#define MPU6050_REG_GYRO_CONFIG 0x1B   // Configuração do giroscópio
#define MPU6050_REG_ACCEL_CONFIG 0x1C  // Configuração do acelerômetro
#define MPU6050_REG_FIFO_EN 0x23       // Habilita buffer FIFO

// Configuração de escala
#define MPU6050_GYRO_CONFIG_FS_250 0x00  // ±250°/s
#define MPU6050_GYRO_CONFIG_FS_500 0x08  // ±500°/s
#define MPU6050_GYRO_CONFIG_FS_1000 0x10 // ±1000°/s
#define MPU6050_GYRO_CONFIG_FS_2000 0x18 // ±2000°/s

#define MPU6050_ACCEL_CONFIG_FS_2G 0x00  // ±2g
#define MPU6050_ACCEL_CONFIG_FS_4G 0x08  // ±4g
#define MPU6050_ACCEL_CONFIG_FS_8G 0x10  // ±8g
#define MPU6050_ACCEL_CONFIG_FS_16G 0x18 // ±16g

// Dados dos sensores
#define MPU6050_REG_ACCEL_XOUT_MSB 0x3B
#define MPU6050_REG_ACCEL_XOUT_LSB 0x3C
#define MPU6050_REG_ACCEL_YOUT_MSB 0x3D
#define MPU6050_REG_ACCEL_YOUT_LSB 0x3E
#define MPU6050_REG_ACCEL_ZOUT_MSB 0x3F
#define MPU6050_REG_ACCEL_ZOUT_LSB 0x40

#define MPU6050_REG_TEMP_OUT_MSB 0x41
#define MPU6050_REG_TEMP_OUT_LSB 0x42

#define MPU6050_REG_GYRO_XOUT_MSB 0x43
#define MPU6050_REG_GYRO_XOUT_LSB 0x44
#define MPU6050_REG_GYRO_YOUT_MSB 0x45
#define MPU6050_REG_GYRO_YOUT_LSB 0x46
#define MPU6050_REG_GYRO_ZOUT_MSB 0x47
#define MPU6050_REG_GYRO_ZOUT_LSB 0x48

#define MPU6050_REG_SIG_PATH_RESET 0x68 // Reset de caminho de sinal

// Configuração de energia
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_PWR_MGMT_2 0x6C

// Registrador de identificação do dispositivo
#define MPU6050_REG_WHO_AM_I 0x75

// Registradores de Offset para calibração
#define MPU6050_REG_XA_OFFSET_MSB 0x06
#define MPU6050_REG_XA_OFFSET_LSB 0x07
#define MPU6050_REG_YA_OFFSET_MSB 0x08
#define MPU6050_REG_YA_OFFSET_LSB 0x09
#define MPU6050_REG_ZA_OFFSET_MSB 0x0A
#define MPU6050_REG_ZA_OFFSET_LSB 0x0B

#define MPU6050_REG_XG_OFFSET_MSB 0x13
#define MPU6050_REG_XG_OFFSET_LSB 0x14
#define MPU6050_REG_YG_OFFSET_MSB 0x15
#define MPU6050_REG_YG_OFFSET_LSB 0x16
#define MPU6050_REG_ZG_OFFSET_MSB 0x17
#define MPU6050_REG_ZG_OFFSET_LSB 0x18

esp_err_t mpu6050_init(mpu6050_dev_t *dev) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t who_am_i;
    if (sensor_read_reg(dev->addr, MPU6050_REG_WHO_AM_I, &who_am_i, 1) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao ler ID do dispositivo");
        return ESP_FAIL;
    }

    if (who_am_i != MPU6050_ADDRESS) {
        Serial.printf("[MPU6050 (Acel/Giro)] ID do dispositivo inválido: 0x%02X\n", who_am_i);
        return ESP_FAIL;
    }
    Serial.printf("[MPU6050 (Acel/Giro)] Dispositivo encontrado: 0x%02X\n", who_am_i);

    if (mpu6050_pwr_mgmt_1_mode(dev, 0, 0, 0, MPU6050_PWR_MGMT_1_CLKSEL_PLL_X) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao configurar modo de energia");
        return ESP_FAIL;
    }

    if (mpu6050_set_filter(dev, dev->filter_config) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao configurar filtro");
        return ESP_FAIL;
    }

    if (mpu6050_accel_set_config(dev, dev->accel_fs) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao configurar acelerômetro");
        return ESP_FAIL;
    }

    if (mpu6050_gyro_set_config(dev, dev->gyro_fs) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao configurar giroscópio");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t mpu6050_set_filter(mpu6050_dev_t *dev, mpu6050_filter_config_t filter_config) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (filter_config > MPU6050_CONFIG_FILTER_5HZ || filter_config < MPU6050_CONFIG_FILTER_260HZ) {
        Serial.println("[MPU6050 (Acel/Giro)] Filtro inválido");
        return ESP_ERR_INVALID_ARG;
    }

    Serial.printf("[MPU6050 (Acel/Giro)] Configurando filtro: 0x%02X\n", filter_config);
    dev->filter_config = filter_config;
    // Configura o filtro no registrador de configuração do filtro
    return sensor_write_reg(dev->addr, MPU6050_REG_FILTER_CONFIG, filter_config);
}

esp_err_t mpu6050_soft_reset(mpu6050_dev_t *dev) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    Serial.println("[MPU6050 (Acel/Giro)] Resetando caminho de sinal");
    return sensor_write_reg(dev->addr, MPU6050_REG_SIG_PATH_RESET, MPU6050_SIG_PATH_RESET_TEMP_RST | MPU6050_SIG_PATH_RESET_ACCEL_RST | MPU6050_SIG_PATH_RESET_GYRO_RST);
}

esp_err_t mpu6050_hard_reset(mpu6050_dev_t *dev) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    Serial.println("[MPU6050 (Acel/Giro)] Resetando o dispositivo");
    return sensor_write_reg(dev->addr, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_MGMT_1_DEVICE_RESET);
}

esp_err_t mpu6050_pwr_mgmt_1_mode(mpu6050_dev_t *dev, uint8_t sleep, uint8_t cycle, uint8_t temp_dis, uint8_t clk_sel) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t pwr_mgmt_1 = sleep | cycle | temp_dis | clk_sel;
    Serial.printf("[MPU6050 (Acel/Giro)] Configurando modo de energia: 0x%02X\n", pwr_mgmt_1);
    return sensor_write_reg(dev->addr, MPU6050_REG_PWR_MGMT_1, pwr_mgmt_1);
}

esp_err_t mpu6050_pwr_mgmt_2_mode(mpu6050_dev_t *dev, uint8_t lp_wake_ctrl, uint8_t stby_xa, uint8_t stby_ya, uint8_t stby_za, uint8_t stby_xg, uint8_t stby_yg, uint8_t stby_zg) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t pwr_mgmt_2 = lp_wake_ctrl | stby_xa | stby_ya | stby_za | stby_xg | stby_yg | stby_zg;
    Serial.printf("[MPU6050 (Acel/Giro)] Configurando modo de energia 2: 0x%02X\n", pwr_mgmt_2);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_PWR_MGMT_2, pwr_mgmt_2);
}

esp_err_t mpu6050_accel_set_config(mpu6050_dev_t *dev, mpu6050_accel_fs_t accel_fs) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (accel_fs > MPU6050_CONFIG_AFS3 || accel_fs < MPU6050_CONFIG_AFS0) {
        Serial.println("[MPU6050 (Acel/Giro)] Configuração de acelerômetro inválida");
        return ESP_ERR_INVALID_ARG;
    }

    dev->accel_fs = accel_fs;
    uint8_t accel_fs_config = (accel_fs << 3) & 0x18; // Configuração do acelerômetro
    Serial.printf("[MPU6050 (Acel/Giro)] Configurando acelerômetro: 0x%02X\n", accel_fs);
    return sensor_write_reg(dev->addr, MPU6050_REG_ACCEL_CONFIG, accel_fs_config);
}

esp_err_t mpu6050_gyro_set_config(mpu6050_dev_t *dev, mpu6050_gyro_fs_t gyro_fs) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (gyro_fs > MPU6050_CONFIG_GFS3 || gyro_fs < MPU6050_CONFIG_GFS0) {
        Serial.println("[MPU6050 (Acel/Giro)] Configuração de giroscópio inválida");
        return ESP_ERR_INVALID_ARG;
    }

    dev->gyro_fs = gyro_fs;
    uint8_t gyro_fs_config = (gyro_fs << 3) & 0x18; // Configuração do giroscópio
    Serial.printf("[MPU6050 (Acel/Giro)] Configurando giroscópio: 0x%02X\n", gyro_fs);
    return sensor_write_reg(dev->addr, MPU6050_REG_GYRO_CONFIG, gyro_fs_config);
}

esp_err_t mpu6050_calibrate_accel(mpu6050_dev_t *dev, uint16_t samples) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (samples == 0) {
        Serial.println("[MPU6050 (Acel/Giro)] Número de amostras inválido");
        return ESP_ERR_INVALID_ARG;
    }

    accel_data_t data = {0};
    for (uint16_t i = 0; i < samples; i++) {
        mpu6050_accel_read_data(dev, &data);
        dev->accel_offset.x += data.x;
        dev->accel_offset.y += data.y;
        dev->accel_offset.z += data.z;
        delay(10);
    }
    dev->accel_offset.x /= samples;
    dev->accel_offset.y /= samples;
    dev->accel_offset.z /= samples;

    Serial.printf("[MPU6050 (Acel/Giro)] Acelerômetro calibrado: x=%.2f, y=%.2f, z=%.2f\n", dev->accel_offset.x, dev->accel_offset.y, dev->accel_offset.z);
    return ESP_OK;
}

esp_err_t mpu6050_calibrate_gyro(mpu6050_dev_t *dev, uint16_t samples) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (samples == 0) {
        Serial.println("[MPU6050 (Acel/Giro)] Número de amostras inválido");
        return ESP_ERR_INVALID_ARG;
    }

    gyro_data_t data;
    for (uint16_t i = 0; i < samples; i++) {
        mpu6050_gyro_read_data(dev, &data);
        dev->gyro_offset.x += data.x;
        dev->gyro_offset.y += data.y;
        dev->gyro_offset.z += data.z;
        delay(10);
    }
    dev->gyro_offset.x /= samples;
    dev->gyro_offset.y /= samples;
    dev->gyro_offset.z /= samples;

    Serial.printf("[MPU6050 (Acel/Giro)] Giroscópio calibrado: x=%.2f, y=%.2f, z=%.2f\n", dev->gyro_offset.x, dev->gyro_offset.y, dev->gyro_offset.z);
    return ESP_OK;
}

esp_err_t mpu6050_accel_read_data(mpu6050_dev_t *dev, accel_data_t *data) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dados do acelerômetro não inicializados");
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->accel_fs > MPU6050_CONFIG_AFS3 || dev->accel_fs < MPU6050_CONFIG_AFS0) {
        Serial.println("[MPU6050 (Acel/Giro)] Configuração de acelerômetro inválida");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[6];
    if (sensor_read_reg(dev->addr, MPU6050_REG_ACCEL_XOUT_MSB, buffer, 6) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao ler dados do acelerômetro");
        return ESP_FAIL;
    }

    uint16_t scale = (1 << (14 - dev->accel_fs));
    int16_t raw = (int16_t)(buffer[0] << 8 | buffer[1]);
    data->x = (float)raw / scale - dev->accel_offset.x;
    raw = (int16_t)(buffer[2] << 8 | buffer[3]);
    data->y = (float)raw / scale - dev->accel_offset.y;
    raw = (int16_t)(buffer[4] << 8 | buffer[5]);
    data->z = (float)raw / scale - dev->accel_offset.z;
    return ESP_OK;
}

esp_err_t mpu6050_gyro_read_data(mpu6050_dev_t *dev, gyro_data_t *data) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (data == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dados do giroscópio não inicializados");
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->gyro_fs > MPU6050_CONFIG_GFS3 || dev->gyro_fs < MPU6050_CONFIG_GFS0) {
        Serial.println("[MPU6050 (Acel/Giro)] Configuração de giroscópio inválida");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[6];
    if (sensor_read_reg(dev->addr, MPU6050_REG_GYRO_XOUT_MSB, buffer, 6) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao ler dados do giroscópio");
        return ESP_FAIL;
    }

    float scale = (1 << (17 - dev->gyro_fs)) / 1000.0f;
    int16_t raw = (int16_t)(buffer[0] << 8 | buffer[1]);
    data->x = (float)raw / scale - dev->gyro_offset.x;
    raw = (int16_t)(buffer[2] << 8 | buffer[3]);
    data->y = (float)raw / scale - dev->gyro_offset.y;
    raw = (int16_t)(buffer[4] << 8 | buffer[5]);
    data->z = (float)raw / scale - dev->gyro_offset.z;
    return ESP_OK;
}

esp_err_t mpu6050_read_temp(mpu6050_dev_t *dev, int16_t *temp) {
    if (dev == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dispositivo não inicializado");
        return ESP_ERR_INVALID_ARG;
    }

    if (temp == NULL) {
        Serial.println("[MPU6050 (Acel/Giro)] Dados de temperatura não inicializados");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[2];
    if (sensor_read_reg(MPU6050_ADDRESS, MPU6050_REG_TEMP_OUT_MSB, buffer, 2) != ESP_OK) {
        Serial.println("[MPU6050 (Acel/Giro)] Falha ao ler temperatura");
        return ESP_FAIL;
    }
    *temp = (int16_t)(buffer[0] << 8 | buffer[1]);
    return ESP_OK;
}