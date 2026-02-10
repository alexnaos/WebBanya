#include "ui.h"
//#include "config/config.h" // Чтобы ui.cpp видел ваши переменные и пины
// В начале ui.cpp
#include <Arduino.h>

extern float currentTemp; // берем из main.cpp


void build(sets::Builder& gui) {
    // Весь ваш код из функции build перенесите сюда
    gui.Label("temp", "Температура");
    // ... и так далее
}

void updateOLED() {
    // Код обновления экрана
}
