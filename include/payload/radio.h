#pragma once

#include "esp_err.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "message_types.h"

#define RADIO_EV_TRANSMITTED (1<<0)

extern EventGroupHandle_t radio_event_group;

esp_err_t radio_init();
esp_err_t radio_transmit_data(radio_message_t *radio_message);