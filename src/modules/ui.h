
#ifndef UI_H
#define UI_H
#include <Arduino.h>
#include "../config/config.h" // Правильно: выход из modules и вход в config


// 2. Теперь можно использовать DeviceAddress, так как он уже известен
extern DeviceAddress addr1; 
extern DeviceAddress addr2;
// Объявления функций, реализованных в ui.cpp
void build(sets::Builder& b);
void updateOLED();

#endif

