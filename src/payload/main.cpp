#include "payload/main.h"

void consume_task(void *args) {
    nav_data_t nav_data;
    amb_data_t amb_data;

    for (;;) {
        if (xQueueReceive(amb_data_queue, &amb_data, pdMS_TO_TICKS(300))) {
            Serial.println("=================================");
            Serial.println("           Ambient Data          ");
            Serial.println("=================================");
            
            Serial.print("Temp: ");
            Serial.print(amb_data.temp);
            Serial.print("°C, Umid: ");
            Serial.print(amb_data.umid);
            Serial.println("%, Press: ");
            Serial.print(amb_data.press);
            Serial.println("hPa");

            Serial.print("mX: ");
            Serial.print(amb_data.mag[0]);
            Serial.print("mT, mY: ");
            Serial.print(amb_data.mag[1]);
            Serial.print("mT, mZ: ");
            Serial.print(amb_data.mag[2]);
            Serial.println("mT");
        }
        if (xQueueReceive(nav_data_queue, &nav_data, pdMS_TO_TICKS(300))) {
            Serial.println("=================================");
            Serial.println("          Navigation Data        ");
            Serial.println("=================================");
            
            Serial.print("Alt: ");
            Serial.print(nav_data.alt);
            Serial.print("m, Vel:");
            Serial.print(nav_data.vel);
            Serial.print("m/s, Head: ");
            Serial.print(nav_data.head);
            Serial.println("°");

            Serial.print("Lat: ");
            Serial.print(nav_data.lati, 6);
            Serial.print(", Long: ");
            Serial.println(nav_data.longi, 6);
        }
    }
}

void nav_task(void *args) {
    baro_data_t baro_data;
    magneto_data_t magneto_data;
    accel_data_t accel_data;
    gyro_data_t gyro_data;

    float azimuth;

    gps_data_t gps_data;

    double avg_alt = 0;

    nav_data_t nav_data;

    for (;;) {
        if (xQueueReceive(baro_nav_queue, &baro_data, pdMS_TO_TICKS(300)) == pdTRUE);
        if (xQueueReceive(magneto_nav_queue, &magneto_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            azimuth = atan2(magneto_data.y,magneto_data.x) * 180.0/PI;
            azimuth += MAG_DECLINATION_ANGLE;
            azimuth += azimuth < 0 ? 360 : 0;

            nav_data.head = azimuth;
        }
        if (xQueueReceive(accel_queue, &accel_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            Serial.println("=================================");
            Serial.println("        Acceleration Data        ");
            Serial.println("=================================");

            Serial.print("aX: ");
            Serial.print(accel_data.x);
            Serial.print("g, aY: ");
            Serial.print(accel_data.y);
            Serial.print("g, aZ: ");
            Serial.print(accel_data.z);
            Serial.println("g");
        }
        if (xQueueReceive(gyro_queue, &gyro_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            Serial.println("=================================");
            Serial.println("          Rotation Data          ");
            Serial.println("=================================");
            
            Serial.print("gX: ");
            Serial.print(gyro_data.x);
            Serial.print("°/s, gY: ");
            Serial.print(gyro_data.y);
            Serial.print("°/s, gZ: ");
            Serial.print(gyro_data.z);
            Serial.println("°/s");
        }
        if (xQueueReceive(gps_queue, &gps_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            nav_data.lati = gps_data.lati;
            nav_data.longi = gps_data.longi;
            nav_data.vel = gps_data.vel;
            nav_data.head = gps_data.head;
        }
        avg_alt = (baro_data.altitude + gps_data.alt) / 2;
        nav_data.alt = avg_alt;
        xQueueSend(nav_data_queue, &nav_data, portMAX_DELAY);
    }
    vTaskDelete(NULL);
}

void amb_task(void *args) {
    baro_data_t baro_data;
    termo_data_t termo_data;
    magneto_data_t magneto_data;

    amb_data_t amb_data;

    float avg_temp = 0;

    for (;;) {
        if (xQueueReceive(baro_amb_queue, &baro_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            amb_data.press = baro_data.pressure;
        }
        if (xQueueReceive(termo_queue, &termo_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            amb_data.umid = termo_data.humd;
        }
        if (xQueueReceive(magneto_amb_queue, &magneto_data, pdMS_TO_TICKS(300)) == pdTRUE) {
            amb_data.mag[0] = magneto_data.x;
            amb_data.mag[1] = magneto_data.y;
            amb_data.mag[2] = magneto_data.z;
        }
        avg_temp = (baro_data.temperature + termo_data.temp) / 2;
        amb_data.temp = avg_temp;
        xQueueSend(amb_data_queue, &amb_data, portMAX_DELAY);
    }
    vTaskDelete(NULL);
}

void termo_task(void *args) {
    termo_data_t data;

    for (;;) {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (termo_read_data(&data) == ESP_OK) {
                xQueueSend(termo_queue, &data, pdMS_TO_TICKS(300));
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

void magneto_task(void *args) {
    magneto_data_t data;

    for (;;) {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (magneto_read_data(&data) == ESP_OK) {
                xQueueSend(magneto_amb_queue, &data, pdMS_TO_TICKS(300));
                xQueueSend(magneto_nav_queue, &data, pdMS_TO_TICKS(300));
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

void accel_gyro_task(void *args) {
    accel_data_t accel_data;
    gyro_data_t gyro_data;

    Serial.println("[Accel/Gyro] Calibrando o sensor");
    int cal_num;
    float gx_cal, gy_cal, gz_cal;
    vTaskDelay(pdMS_TO_TICKS(20));
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
    Serial.println("[Accel/Gyro] Sensor calibrado");

    for (;;) {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            if (accel_read_data(&accel_data) == ESP_OK) {
                accel_data.x = accel_data.x / 8192;
                accel_data.y = accel_data.y / 8192 - 0.01;
                accel_data.z = accel_data.z / 8192 - 0.17;
            
                xQueueSend(accel_queue, &accel_data, pdMS_TO_TICKS(300));
            }
            if (gyro_read_data(&gyro_data) == ESP_OK) {
                gyro_data.x /= 65.5;
                gyro_data.y /= 65.5;
                gyro_data.z /= 65.5;

                xQueueSend(gyro_queue, &gyro_data, pdMS_TO_TICKS(300));
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

void baro_task(void *args) {
    baro_data_t data;

    Serial.println("[Baro] Sensor calibrado");

    for (;;) {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            data.pressure = bmp.readPressure() / 100.0;
            data.temperature = bmp.readTemperature();
            data.altitude = bmp.readAltitude();
            xQueueSend(baro_nav_queue, &data, pdMS_TO_TICKS(300));
            xQueueSend(baro_amb_queue, &data, pdMS_TO_TICKS(300));
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

void gps_task(void *args) {
    gps_data_t gps_data;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(100));
        while (GPS_SERIAL.available() > 0){
            byte gpsData = GPS_SERIAL.read();
            gps.encode(gpsData);
            if (gps.location.isUpdated()) {
                gps_data.lati = gps.location.lat();
                gps_data.longi = gps.location.lng();
            }
            if (gps.speed.isUpdated()) {
                gps_data.vel = gps.speed.mps();
            }
            if (gps.course.isUpdated()) {
                gps_data.head = gps.course.deg();
            }
            if (gps.altitude.isUpdated()) {
                gps_data.alt = gps.altitude.meters();
            }
        }
        xQueueSend(gps_queue, &gps_data, pdMS_TO_TICKS(300));
    }
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    GPS_SERIAL.begin(9600, SERIAL_8N1, GPS_UART_RX_PIN, GPS_UART_TX_PIN);
    Wire.begin();
    
    task_com_init();
    
    if (termo_init() != ESP_OK) {
        Serial.println("[Termo] Falha ao inicializar o sensor");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    if (magneto_init() != ESP_OK) {
        Serial.println("[Magneto] Falha ao inicializar o sensor");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    if (accel_gyro_init() != ESP_OK) {
        Serial.println("[Accel/Gyro] Falha ao inicializar o sensor");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    if (!bmp.begin()) {
    	Serial.println("[Baro] Falha ao inicializar o sensor");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(termo_task, "[Termo] Task", 2048, NULL, 7, NULL);
    xTaskCreate(magneto_task, "[Magneto] Task", 2048, NULL, 5, NULL);
    xTaskCreate(accel_gyro_task, "[Accel/Gyro] Task", 2048, NULL, 6, NULL);
    xTaskCreate(baro_task, "[Baro] Task", 2048, NULL, 5, NULL);
    xTaskCreate(gps_task, "[GPS] Task", 2048, NULL, 4, NULL);
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(nav_task, "[Nav] Task", 2048, NULL, 2, NULL);
    xTaskCreate(amb_task, "[Amb] Task", 2048, NULL, 2, NULL);
    xTaskCreate(consume_task, "[Cons] Task", 2048, NULL, 1, NULL);
}

void loop() {}