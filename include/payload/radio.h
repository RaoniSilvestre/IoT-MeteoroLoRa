#pragma once

#include "esp_err.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#define RADIO_EV_TRANSMITTED (1<<0)

EventGroupHandle_t radio_event_group;

esp_err_t radio_init();
esp_err_t radio_start_receiving();
esp_err_t radio_standby();
esp_err_t radio_receive_data();
void radio_get_status();