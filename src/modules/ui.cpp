#include "../config/config.h"
#include "ui.h"

void build(sets::Builder& b) {
    // Весь ваш код build() здесь
    if (b.beginGroup("Group 1")) {
       // ...
    }
    // ... и т.д. ...
}

void updateOLED() {
    // Весь ваш код updateOLED() здесь
    display.clearDisplay();
    // ...
}
