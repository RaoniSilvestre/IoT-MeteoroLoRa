#include <Arduino.h>
#include <Wire.h>
#include "payload/magneto.h"
#include "payload/config.h"

magneto_data_t *data = (magneto_data_t*)malloc(sizeof(magneto_data_t));
float azimuth;

void magneto_task(void *args) {
    

    for (;;) {
        if (magneto_read_data(data) == ESP_OK) {
            azimuth = atan2(data->y,data->x) * 180.0/PI;
            azimuth += MAG_DECLINATION_ANGLE;
            azimuth += azimuth < 0 ? 360 : 0;


            Serial.printf("X: %d, Y: %d, Z: %d, Heading: %.2fº\n", data->x, data->y, data->z, azimuth);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    if (magneto_init() != ESP_OK) {
        Serial.println("Falha ao inicializar o magneto");
    }
    //xTaskCreate(magneto_task, "Magneto Task", 1024, NULL, 5, NULL);
}

void loop() {
    if (magneto_read_data(data) == ESP_OK) {
        azimuth = atan2(data->y,data->x) * 180.0/PI;
        azimuth += MAG_DECLINATION_ANGLE;
        azimuth += azimuth < 0 ? 360 : 0;


        Serial.printf("X: %d, Y: %d, Z: %d, Heading: %.2fº\n", data->x, data->y, data->z, azimuth);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}
