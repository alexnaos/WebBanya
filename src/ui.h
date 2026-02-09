
#include <Arduino.h>
#include "../config/config.h" // Правильно: выход из modules и вход в config
#ifndef UI_H
#define UI_H

DeviceAddress addr1 = {0x28, 0xB2, 0x54, 0x7F, 0x00, 0x00, 0x00, 0xCF};
DeviceAddress addr2 = {0x28, 0x39, 0xE2, 0x6E, 0x01, 0x00, 0x00, 0x12};
uint32_t tmr;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Глобальные переменные для управления прокруткой
 String longMessage = "";
 int16_t textX = 128; // Начальная позиция
// Объявления функций, реализованных в ui.cpp

void build(sets::Builder& b);
void updateOLED();



#endif

