#include <Arduino.h>
#include <Wire.h>
#include "payload/magneto.h"
#include "payload/termo.h"
#include "payload/config.h"


magneto_data_t *mag_data = (magneto_data_t*)malloc(sizeof(magneto_data_t));
float azimuth;
termo_data_t *termo_data = (termo_data_t*)malloc(sizeof(termo_data_t));
/*
void magneto_task(void *args) {
    

    for (;;) {
        if (magneto_read_data(mag_data) == ESP_OK) {
            azimuth = atan2(mag_data->y,mag_data->x) * 180.0/PI;
            azimuth += MAG_DECLINATION_ANGLE;
            azimuth += azimuth < 0 ? 360 : 0;

            Serial.printf("X: %d, Y: %d, Z: %d, Heading: %.2fº\n", mag_data->x, mag_data->y, mag_data->z, azimuth);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}*/

void setup() {
    Serial.begin(115200);
    Wire.begin();
    if (magneto_init() != ESP_OK) {
        Serial.println("Falha ao inicializar o magneto");
    }
    if (termo_init() != ESP_OK) {
        Serial.println("Falha ao inicializar o termometro");
    }
    //xTaskCreate(magneto_task, "Magneto Task", 1024, NULL, 5, NULL);
}

void loop() {
    int16_t x = 0, y = 0, z = 0;
    float temp = 0, humd = 0;

    for (int i = 0; i < 10; i++) {
        if (magneto_read_data(mag_data) == ESP_OK) {
            x += mag_data->x;
            y += mag_data->y;
            z += mag_data->z;
        }
        if (termo_read_data(termo_data) == ESP_OK) {
            temp += termo_data->temp;
            humd += termo_data->humd;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    mag_data->x = x / 10;
    mag_data->y = y / 10;
    mag_data->z = z / 10;
    termo_data->temp = temp / 10;
    termo_data->humd = humd / 10;
    azimuth = atan2(mag_data->y,mag_data->x) * 180.0/PI;
    azimuth += MAG_DECLINATION_ANGLE;
    azimuth += azimuth < 0 ? 360 : 0;
    Serial.printf("X: %d, Y: %d, Z: %d, Heading: %.2fº\n", mag_data->x, mag_data->y, mag_data->z, azimuth);
    Serial.printf("Temperatura: %.2fºC, Umidade: %.2f%%\n", termo_data->temp, termo_data->humd);
    vTaskDelay(pdMS_TO_TICKS(100));
}
