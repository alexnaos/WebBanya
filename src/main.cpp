 
#include <Arduino.h>
#include "config/config.h"
#include "modules/ui.h"
// ФИЗИЧЕСКОЕ СОЗДАНИЕ ОБЪЕКТА (ОПРЕДЕЛЕНИЕ)
// Имя "sett" должно совпадать с тем, что вы написали в config.h после extern
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Sloboda43", &db);

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("LED is ON");

    // 1. Файловая система (LittleFS) — основа всего
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS Error!");
    }
    // 1. Питание дисплея
    pinMode(DISPLAY_VCC_PIN, OUTPUT);
    digitalWrite(DISPLAY_VCC_PIN, LOW);
    delay(200);
    digitalWrite(DISPLAY_VCC_PIN, HIGH);
    delay(200); // Даем дисплею "проснуться"
                // 2. Файловая система и БД (СТРОГО ДО sett.begin)
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif
    db.begin();
    // Инициализируем базу (только если ключей еще нет)
    db.init(kk::txt, "text");
    db.init(kk::tmp1, 0.0f);
    db.init(kk::tmp2, 0.0f);
    db.init(kk::pass, "some pass");
    db.init(kk::uintw, 64u);
    db.init(kk::intw, -10);
    db.init(kk::int64w, 1234567ll);
    db.init(kk::color, 0xff0000);
    db.init(kk::toggle, true);
    db.init(kk::slider, -3.5);
    db.init(kk::selectw, (uint8_t)1);
    db.init(kk::date, 1719941932);
    db.init(kk::timew, 60);
    db.init(kk::datime, 1719941932);
    db.init(kk::sldmin, -5);
    db.init(kk::sldmax, 5);
    setStampZone(3);
     
    // 4. Сеть
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // ... цикл подключения ...
    WiFi.softAP("AP ESP");

    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.onBuild(build);
    //sett.onUpdate(update);
    sett.config.theme = sets::Colors::Green;
   
}


void loop()
{
    sett.tick();
    // checkSensors();
    // updateOLED();
}
