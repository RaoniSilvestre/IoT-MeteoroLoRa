#include <Arduino.h>
#include "payload/radio.h"

void setup() {
    Serial.begin(115200);
    Serial.println(esp_err_to_name(radio_init()));
}

void loop() {
    radio_message_t data = {MSG_TYPE_PRESSURE, "Olá Mundo!."};
    if (radio_transmit_data(&data) == ESP_OK) {
        Serial.println("Dados transmitidos: " + data.data);
    }
    delay(1000);
}
