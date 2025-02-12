#pragma once

#include "esp_err.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "message_types.h"

#define RADIO_EV_RECIEVED (1<<0)

typedef struct radio_status_t {
    float RSSI;
    float SNR;
    float freq_error;
} radio_status_t;

extern EventGroupHandle_t radio_event_group;
extern QueueHandle_t radio_status_queue;
extern QueueHandle_t radio_data_queue;

esp_err_t radio_init();
esp_err_t radio_start_receiving();
esp_err_t radio_standby();
esp_err_t radio_receive_data();
void radio_get_status();