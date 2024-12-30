#pragma once

#include <stdint.h>

#define LOGO_TIME_MS 2000

void oled_init();
void oled_print(char *str, uint8_t x, uint8_t y);
void oled_print(int num, uint8_t x, uint8_t y);
void oled_print(float num, uint8_t x, uint8_t y);