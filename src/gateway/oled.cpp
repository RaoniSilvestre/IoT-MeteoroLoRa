#include <Arduino.h>
#include <U8g2lib.h>
#include "gateway/config.h"
#include "gateway/oled.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, OLED_SCL_PIN, OLED_SDA_PIN);

void oled_init() {
	printf("Inicializando o display OLED");
	display.begin();
	//display.drawXBM(x, y, w, h, logo);
	display.sendBuffer();
	vTaskDelay(LOGO_TIME_MS);
}

void oled_print(char *str, uint8_t x, uint8_t y) {
	display.clearBuffer();
	display.setCursor(x, y);
	display.print(str);
}

void oled_print(int num, uint8_t x, uint8_t y) {
	display.clearBuffer();
	display.setCursor(x, y);
	display.print(num);
}

void oled_print(float num, uint8_t x, uint8_t y) {
	display.clearBuffer();
	display.setCursor(x, y);
	display.print(num);
}