#include <Arduino.h>
#include <RadioLib.h>
#include "esp_check.h"
#include "freertos/event_groups.h"
#include "gateway/radio.h"
#include "gateway/config.h"

SPIClass spi_bus;
SPISettings spi_settings;

SX1276 radio = new Module(LORA_CS_PIN, LORA_DIO0_PIN, LORA_RST_PIN, LORA_DIO1_PIN, spi_bus, spi_settings);

void _radio_recieve_ISR() {
    BaseType_t task_woken = pdFALSE;
    
    if (xEventGroupSetBitsFromISR(radio_event_group, RADIO_EV_RECIEVED, &task_woken) != pdFAIL) {
        portYIELD_FROM_ISR(task_woken);
    }
}

esp_err_t radio_init() {
    esp_err_t ret = ESP_OK;

    radio_event_group = xEventGroupCreate();

    spi_bus.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN);

    int status = radio.begin();

    ESP_GOTO_ON_FALSE(status == RADIOLIB_ERR_NONE, ESP_FAIL, err, "LoRa Radio", "Falha ao iniciar controlador LoRa.");

    radio.setDio0Action(_radio_recieve_ISR, RISING);

    status = radio.setFrequency(LORA_FREQ_MHZ);

    ESP_GOTO_ON_FALSE(status == RADIOLIB_ERR_NONE, ESP_FAIL, err_freq, "LoRa Radio", "Falha ao configurar a frequência do controlador LoRa.");

    return ESP_OK;
err:
    // XXX Pânico
    Serial.printf("Falha ao iniciar controlador LoRa, status: %d\n", status);
    return ret;
err_freq:
    // XXX Pânico
    Serial.printf("Falha ao configurar a frequência do controlador LoRa, status: %d\n", status);
    return ret;
}

esp_err_t radio_start_receiving() {
    esp_err_t ret = ESP_OK;

    int status = radio.startReceive();

    ESP_GOTO_ON_FALSE(status == RADIOLIB_ERR_NONE, ESP_FAIL, err, "LoRa Radio", "Falha ao iniciar a recepção do controlador LoRa.");

err:
    // XXX Pânico
    Serial.printf("Falha ao iniciar a recepção do controlador LoRa, status: %d\n", status);
    return ret;
}

esp_err_t radio_receive_data(String data, radio_status_t *radio_status) {
    esp_err_t ret = ESP_OK;

    int status = radio.readData(data);

    ESP_GOTO_ON_FALSE(status == RADIOLIB_ERR_NONE, ESP_FAIL, err, "LoRa Radio", "Falha ao receber pacote LoRa.");

    radio_get_status(radio_status);
    
    return ESP_OK;

err:
    if (status == RADIOLIB_ERR_CRC_MISMATCH) {
        Serial.printf("Pacote LoRa corrompido recebido, dados descartados.");
    } else {
        Serial.printf("Falha ao receber pacote LoRa, status %d", status);
    }
    return ret;
}

void radio_get_status(radio_status_t *radio_status) {
    radio_status->RSSI = radio.getRSSI();
    radio_status->SNR = radio.getSNR();
    radio_status->freq_error = radio.getFrequencyError();
}