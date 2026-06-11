#include "config.h"
#include <ArduinoOTA.h>

void initOTA()
{
    ArduinoOTA.setHostname(HA_DEVICE_NAME);
    ArduinoOTA.setPassword("admin"); // можно сменить на свой

    ArduinoOTA.onStart([]() {
        Serial.println("OTA: Start");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("OTA: End");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA: Progress %u%%\r", (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA: Error %u\n", error);
    });

    ArduinoOTA.begin();
    Serial.println("OTA ready on port 8266");
}

void handleOTA()
{
    ArduinoOTA.handle();
}