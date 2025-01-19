#include "payload/accel_gyro.h"
#include "payload/i2c_sensor.h"
#include <Arduino.h>

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

esp_err_t accel_gyro_init() {
    if (accel_gyro_pwr_mgmt_1_mode(0x00, 0x00, 0x00, MPU6050_PWR_MGMT_1_CLKSEL_PLL_X) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao configurar o modo de energia");
        return ESP_FAIL;
    }
    if (accel_gyro_set_filter(MPU6050_FILTER_CONFIG_5HZ) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao configurar o filtro");
        return ESP_FAIL;
    }
    if (accel_set_config(MPU6050_ACCEL_CONFIG_FS_4G) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao configurar o acelerômetro");
        return ESP_FAIL;
    }
    if (gyro_set_config(MPU6050_GYRO_CONFIG_FS_500) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao configurar o giroscópio");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t accel_gyro_set_filter(uint8_t filter_config) {
    Serial.printf("[Accel/Gyro] Configurando filtro: 0x%02X\n", filter_config);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_FILTER_CONFIG, filter_config);
}

esp_err_t accel_gyro_soft_reset() {
    Serial.println("[Accel/Gyro] Resetando caminho de sinal");
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_SIG_PATH_RESET, MPU6050_SIG_PATH_RESET_TEMP_RST | MPU6050_SIG_PATH_RESET_ACCEL_RST | MPU6050_SIG_PATH_RESET_GYRO_RST);
}

esp_err_t accel_gyro_hard_reset() {
    Serial.println("[Accel/Gyro] Resetando o dispositivo");
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_MGMT_1_DEVICE_RESET);
}

esp_err_t accel_gyro_pwr_mgmt_1_mode(uint8_t sleep, uint8_t cycle, uint8_t temp_dis, uint8_t clk_sel) {
    uint8_t pwr_mgmt_1 = sleep | cycle | temp_dis | clk_sel;
    Serial.printf("[Accel/Gyro] Configurando modo de energia: 0x%02X\n", pwr_mgmt_1);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_PWR_MGMT_1, pwr_mgmt_1);
}

esp_err_t accel_gyro_pwr_mgmt_2_mode(uint8_t lp_wake_ctrl, uint8_t stby_xa, uint8_t stby_ya, uint8_t stby_za, uint8_t stby_xg, uint8_t stby_yg, uint8_t stby_zg) {
    uint8_t pwr_mgmt_2 = lp_wake_ctrl | stby_xa | stby_ya | stby_za | stby_xg | stby_yg | stby_zg;
    Serial.printf("[Accel/Gyro] Configurando modo de energia 2: 0x%02X\n", pwr_mgmt_2);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_PWR_MGMT_2, pwr_mgmt_2);
}

esp_err_t accel_set_config(uint8_t accel_fs) {
    Serial.printf("[Accel/Gyro] Configurando acelerômetro: 0x%02X\n", accel_fs);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_ACCEL_CONFIG, accel_fs);
}

esp_err_t gyro_set_config(uint8_t gyro_fs) {
    Serial.printf("[Accel/Gyro] Configurando giroscópio: 0x%02X\n", gyro_fs);
    return sensor_write_reg(MPU6050_ADDRESS, MPU6050_REG_GYRO_CONFIG, gyro_fs);
}

esp_err_t accel_read_data(accel_data_t *data) {
    uint8_t buffer[6];
    if (sensor_read_reg(MPU6050_ADDRESS, MPU6050_REG_ACCEL_XOUT_MSB, buffer, 6) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao ler dados do acelerômetro");
        return ESP_FAIL;
    }
    int16_t raw = (int16_t)(buffer[0] << 8 | buffer[1]);
    data->x = (float)raw;
    raw = (int16_t)(buffer[2] << 8 | buffer[3]);
    data->y = (float)raw;
    raw = (int16_t)(buffer[4] << 8 | buffer[5]);
    data->z = (float)raw; 
    return ESP_OK;
}

esp_err_t gyro_read_data(gyro_data_t *data) {
    uint8_t buffer[6];
    if (sensor_read_reg(MPU6050_ADDRESS, MPU6050_REG_GYRO_XOUT_MSB, buffer, 6) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao ler dados do giroscópio");
        return ESP_FAIL;
    }
    int16_t raw = (int16_t)(buffer[0] << 8 | buffer[1]);
    data->x = (float)raw;
    raw = (int16_t)(buffer[2] << 8 | buffer[3]);
    data->y = (float)raw;
    raw = (int16_t)(buffer[4] << 8 | buffer[5]);
    data->z = (float)raw;
    return ESP_OK;
}

esp_err_t accel_gyro_read_temp(int16_t *temp) {
    uint8_t buffer[2];
    if (sensor_read_reg(MPU6050_ADDRESS, MPU6050_REG_TEMP_OUT_MSB, buffer, 2) != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao ler temperatura");
        return ESP_FAIL;
    }
    *temp = (int16_t)(buffer[0] << 8 | buffer[1]);
    return ESP_OK;
}