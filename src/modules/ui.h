#include <Arduino.h>
#ifndef UI_H
#define UI_H

#include "../config/config.h" // Подключаем общие объявления и библиотеки
// Только ОБЪЯВЛЕНИЯ:
//extern LiquidCrystal_I2C display; 
DeviceAddress addr1 = {0x28, 0xB2, 0x54, 0x7F, 0x00, 0x00, 0x00, 0xCF};
DeviceAddress addr2 = {0x28, 0x39, 0xE2, 0x6E, 0x01, 0x00, 0x00, 0x12};

extern float temp1; // <-- Добавьте эту строку
extern float temp2; // <-- Добавьте эту строку

// Объявления функций, реализованных в ui.cpp
void build(sets::Builder& b);
void updateOLED();
void setupUI(); // Прототип функции

#endif

