#include <Arduino.h>
#include "gateway/oled.h"

void setup() {
    Serial.begin(115200);
    oled_init();
}

void loop() {
    oled_print("Olá Mundo!", 0, 0);
}
