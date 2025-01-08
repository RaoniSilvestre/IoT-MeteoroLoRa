#include <Arduino.h>
#include <LoRa_E220.h>
#include "esp_check.h"
#include "freertos/event_groups.h"
#include "payload/radio.h"
#include "payload/config.h"

LoRa_E220 radio(&Serial2, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN);

esp_err_t radio_init() {
    esp_err_t ret = ESP_OK;

    if (!radio.begin()) {
        Serial.printf("Falha ao iniciar comunicação com controlador LoRa.\n");
        return ESP_FAIL;
    }
    
    ResponseStructContainer device_data = radio.getConfiguration();
    Configuration config = *(Configuration*) device_data.data;
    // XXX Mexer nas configurações (talvez?)
    config.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
    device_data.close();

    return ESP_OK;
}

esp_err_t radio_transmit(radio_message_t *radio_message) {
    esp_err_t ret = ESP_OK;
    
    ResponseStatus rs = radio.sendBroadcastFixedMessage(LORA_CHANNEL, radio_message, sizeof(radio_message));

    ESP_GOTO_ON_FALSE(rs.code == E220_SUCCESS, ESP_FAIL, err, "LoRa Radio", "Falha ao transmitir mensagem via rádio.");

    return ESP_OK;
err:
    // XXX Pânico
    Serial.printf("Falha ao transmitir mensagem via rádio, status: %d\n", rs.code);
    return ret;
}