#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "freertos/event_groups.h"

#define RADIO_EV_RECIEVED (1<<0)

typedef struct radio_status_t {
    float RSSI;
    float SNR;
    float freq_error;
} radio_status_t;

EventGroupHandle_t radio_event_group;

esp_err_t radio_init();
esp_err_t radio_start_receiving();
esp_err_t radio_receive_data(String data, radio_status_t *radio_status);
void radio_get_status(radio_status_t *radio_status);