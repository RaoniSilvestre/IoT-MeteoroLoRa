#include "payload/tasks.h"

#include "payload/magneto.h"
#include "payload/termo.h"
#include "payload/accel_gyro.h"
#include "payload/baro.h"
#include "payload/config.h"

QueueHandle_t accel_queue;
QueueHandle_t gyro_queue;
QueueHandle_t magneto_nav_queue;
QueueHandle_t magneto_amb_queue;
QueueHandle_t termo_queue;
QueueHandle_t baro_nav_queue;
QueueHandle_t baro_amb_queue;

QueueHandle_t nav_data_queue;
QueueHandle_t amb_data_queue;

SemaphoreHandle_t i2c_mutex;

void task_com_init() {
    accel_queue = xQueueCreate(5, sizeof(accel_data_t));
    gyro_queue = xQueueCreate(5, sizeof(gyro_data_t));
    magneto_amb_queue = xQueueCreate(5, sizeof(magneto_data_t));
    magneto_nav_queue = xQueueCreate(5, sizeof(magneto_data_t));
    termo_queue = xQueueCreate(5, sizeof(termo_data_t));
    baro_amb_queue = xQueueCreate(5, sizeof(baro_data_t));
    baro_nav_queue = xQueueCreate(5, sizeof(baro_data_t));

    nav_data_queue = xQueueCreate(5, sizeof(nav_data_t));
    amb_data_queue = xQueueCreate(5, sizeof(amb_data_t));

    i2c_mutex = xSemaphoreCreateMutex();
}