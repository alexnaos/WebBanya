#include "../config/config.h"
#include "automation.h"

void runAutomation() {
    // Ваш код runAutomation() здесь
    String jsonStr = db[kk::logic].toString();
    JsonDocument doc;
    // ... ваш код парсинга JSON и управления реле ...
    if (deserializeJson(doc, jsonStr) == DeserializationError::Ok) {
       // ...
    }
}

void loadLogicFromFile() {
    // Ваш код loadLogicFromFile() здесь
    if (LittleFS.exists("/logic.json")) {
       // ...
    }
}
