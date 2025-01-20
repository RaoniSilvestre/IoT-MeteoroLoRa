#pragma once

#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

typedef struct {
    float temp;
    float umid;
    float press;
    float mag[3];
} amb_data_t;

typedef struct {
    float alt;
    float vel;
    float head;
    String lati, longi;
} nav_data_t;

extern QueueHandle_t accel_queue;
extern QueueHandle_t gyro_queue;
extern QueueHandle_t magneto_nav_queue;
extern QueueHandle_t magneto_amb_queue;
extern QueueHandle_t termo_queue;
extern QueueHandle_t baro_nav_queue;
extern QueueHandle_t baro_amb_queue;

extern QueueHandle_t nav_data_queue;
extern QueueHandle_t amb_data_queue;

extern SemaphoreHandle_t i2c_mutex;

void task_com_init();