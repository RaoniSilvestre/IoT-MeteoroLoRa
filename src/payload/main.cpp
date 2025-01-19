#include <Arduino.h>
#include <Wire.h>
#include "payload/magneto.h"
#include "payload/termo.h"
#include "payload/accel_gyro.h"
#include "payload/config.h"

void magneto_task(void *args) {
    magneto_data_t mag_data;
    float azimuth;
    termo_data_t termo_data;

    for (;;) {
        int16_t x = 0, y = 0, z = 0;
        float temp = 0, humd = 0;

        for (int i = 0; i < 10; i++) {
            if (magneto_read_data(&mag_data) == ESP_OK) {
                x += mag_data.x;
                y += mag_data.y;
                z += mag_data.z;
            }
            if (termo_read_data(&termo_data) == ESP_OK) {
                temp += termo_data.temp;
                humd += termo_data.humd;
            }
            //vTaskDelay(pdMS_TO_TICKS(100));
        }
        mag_data.x = x / 10;
        mag_data.y = y / 10;
        mag_data.z = z / 10;

        termo_data.temp = temp / 10;
        termo_data.humd = humd / 10;

        azimuth = atan2(mag_data.y,mag_data.x) * 180.0/PI;
        azimuth += MAG_DECLINATION_ANGLE;
        azimuth += azimuth < 0 ? 360 : 0;

        Serial.print("X: ");
        Serial.print(mag_data.x);
        Serial.print("mT, Y: ");
        Serial.print(mag_data.y);
        Serial.print("mT, Z: ");
        Serial.print(mag_data.z);
        Serial.print("mT, Heading: ");
        Serial.print(azimuth);
        Serial.println("º");

        Serial.print("Temp: ");
        Serial.print(termo_data.temp);
        Serial.print("ºC, Umid: ");
        Serial.print(termo_data.humd);
        Serial.println("%");
    }
    vTaskDelete(NULL);
}

void accel_gyro_task(void *args) {
    accel_data_t accel_data;
    gyro_data_t gyro_data;

    float gx_cal, gy_cal, gz_cal;
    int cal_num;

    for (cal_num = 1; cal_num <= 500; cal_num++) {
        if (gyro_read_data(&gyro_data) == ESP_OK) {
            gx_cal += gyro_data.x / 65.5;
            gy_cal += gyro_data.y / 65.5;
            gz_cal += gyro_data.z / 65.5;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    gx_cal /= cal_num;
    gy_cal /= cal_num;
    gz_cal /= cal_num;

    for (;;) {
        int32_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
        for (int i = 0; i < 10; i++) {
            if (accel_read_data(&accel_data) == ESP_OK) {
                ax += accel_data.x;
                ay += accel_data.y;
                az += accel_data.z;
            }
            if (gyro_read_data(&gyro_data) == ESP_OK) {
                gx += gyro_data.x;
                gy += gyro_data.y;
                gz += gyro_data.z;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        accel_data.x = ax / 10;
        accel_data.y = ay / 10;
        accel_data.z = az / 10;

        gyro_data.x = gx / 10;
        gyro_data.y = gy / 10;
        gyro_data.z = gz / 10;

        accel_data.x = accel_data.x / 8192;
        accel_data.y = accel_data.y / 8192 - 0.01;
        accel_data.z = accel_data.z / 8192 - 0.17;
        
        gyro_data.x /= 65.5;
        gyro_data.y /= 65.5;
        gyro_data.z /= 65.5;

        gyro_data.x -= gx_cal;
        gyro_data.y -= gy_cal;
        gyro_data.z -= gz_cal;

        Serial.print("Acelerometro: X: ");
        Serial.print(accel_data.x);
        Serial.print("g, Y: ");
        Serial.print(accel_data.y);
        Serial.print("g, Z: ");
        Serial.print(accel_data.z);
        Serial.println("g");
        
        Serial.print("Giroscópio: X: ");
        Serial.print(gyro_data.x);
        Serial.print("°/s Y: ");
        Serial.print(gyro_data.y);
        Serial.print("°/s Z: ");
        Serial.print(gyro_data.z);
        Serial.println("°/s");
        
        //if (accel_read_data(&accel_data) == ESP_OK && gyro_read_data(&gyro_data) == ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(90));
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    if (magneto_init() != ESP_OK) {
        Serial.println("[Magneto] Falha ao inicializar o sensor");
    }
    if (termo_init() != ESP_OK) {
        Serial.println("[Termo] Falha ao inicializar o sensor");
    }
    xTaskCreate(magneto_task, "Magneto Task", 2048, NULL, 5, NULL);
    /*if (accel_gyro_init() != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao inicializar o sensor");
    }*/

    //xTaskCreate(accel_gyro_task, "Accel Task", 2048, NULL, 5, NULL);
}

void loop() {
    
}
