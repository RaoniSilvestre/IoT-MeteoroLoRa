#include <Arduino.h>
#include "gateway/radio.h"

void radio_task(void *args) {
    EventBits_t radio_bit;

    radio_message_t *buffer;

    Serial.println("Task");

    for (;;) {
        radio_bit = xEventGroupWaitBits(radio_event_group, RADIO_EV_RECIEVED, pdTRUE, pdTRUE, pdMS_TO_TICKS(300));

        if (radio_bit & RADIO_EV_RECIEVED) {
            radio_receive_data();
            if (xQueueReceive(radio_data_queue, buffer, portMAX_DELAY) == pdTRUE) {
                Serial.print("Mensagem recebida: " + buffer->data);
            }
        } else {
            ESP_LOGI("LoRa Radio", "Nenhuma mensagem recebida!");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);
    Serial.println(esp_err_to_name(radio_init()));
    xTaskCreate(radio_task, "Radio Task", 1024, NULL, 3, NULL);
}

void loop() {
    
}
