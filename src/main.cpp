#include <Arduino.h>
#include "config.h"

GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett(HA_DEVICE_NAME, &db);
sets::Logger logger(200);

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    analogWriteRange(PWM_MAX);
    analogWriteFreq(PWM_FREQ);
    analogWrite(LED_PIN, 0);

    LittleFS.begin();
    db.begin();

    // Инициализация ключей БД (только если отсутствуют)
    db.init(kk::txt, "text");
    db.init(kk::tmp1, 0.0f);
    db.init(kk::tmp2, 0.0f);
    db.init(kk::toggle, true);
    db.init(kk::slider, -3.5);

    setStampZone(TIMEZONE);

    // Инициализация периферии
    Wire.begin();
    initDisplay();
    initRTC();
    initSensors();

    // Wi-Fi
    initWiFi();

    // MQTT (после WiFi)
    initMQTT();

    // OTA (после WiFi)
    initOTA();

    // Веб-интерфейс (после БД)
    sett.begin();
    sett.config.theme = sets::Colors::Green;
    sett.onBuild(build);
    sett.onUpdate(update);

    pinMode(LED_PIN, OUTPUT);
    Serial.println("Setup done");
}

void loop()
{
    // Проверка millis() переполнения — безопасно с uint32_t
    wifiMulti.run();

    sett.tick();
    db.tick();
    handleSensors();
    handleControl();
    handleMQTT();

    // OLED обновление
    static uint32_t tmrOled;
    if (millis() - tmrOled >= OLED_UPDATE_MS)
    {
        tmrOled = millis();
        updateOLED();
    }

    // OTA
    handleOTA();
}