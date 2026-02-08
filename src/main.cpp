#include <Arduino.h>
#include <LittleFS.h>
#include "config/config.h" // Подключаем общие объявления

// Объявление глобальных объектов здесь
GyverDS3231 rtc;
// ... остальные объекты ...
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett;

void setup() {
    // Ваш setup() код
    Serial.begin(115200);
    LittleFS.begin();
    db.begin();
    // ... вызовы loadLogicFromFile(); ...
}

void loop() {
    // Ваш loop() код
    sett.tick();
    // ... таймеры и вызовы updateOLED(); ...
}
