
#include <Arduino.h>
#include "config.h"
#include "ui_portal.h"
// Создание объектов
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
GyverDS3231 rtc;
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Slobanya3 разделен!", &db);
sets::Logger logger(200);

void initTime()
{
    rtc.begin();
    uint32_t unix = rtc.GyverDS3231Min::getUnix();
    // Синхронизируем всё при старте
    timeval tv = {(time_t)unix, 0};
    settimeofday(&tv, NULL);
    setStampZone(3);
    sett.rtc = unix;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    // 1. Конфигурация пинов
    pinMode(D7, OUTPUT);
    analogWriteRange(1023);
    analogWriteFreq(20000);
    analogWrite(D7, 0);
    Serial.println("LED is ON");
    LittleFS.begin();
    db.begin();
    // Инициализируем базу (только если ключей еще нет)
    db.init(kk::txt, "text");
    db.init(kk::tmp1, 0.0f);
    db.init(kk::tmp2, 0.0f);
    db.init(kk::toggle, true);
    db.init(kk::slider, -3.5);
    setStampZone(3);
    // Проверяем, создана ли база
    if (!db.has(kk::logic))
    {
        db[kk::logic] = "{}";
        db.update();
    }
    // 3. Железо
    Wire.begin();
    initTime(); // Теперь setup выглядит аккуратно
    initSensors();
    // 4. Сеть
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // ... цикл подключения ...
    WiFi.softAP("AP ESP");
    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.config.theme = sets::Colors::Green;
    setStampZone(3); // Часовой пояс
    sett.onBuild(build);
    sett.onUpdate(update);
}
void loop()
{
    sett.tick();     // Обслуживание веб-интерфейса (ОБЯЗАТЕЛЬНО)
    db.tick();       // Обслуживание базы данных (сохранение на диск)
    handleSensors(); // Опрос датчиков (внутри функции свой таймер)

    // --- 2. Управление нагрузкой (ШИМ на D7) ---
    static uint32_t tmrControl;
    if (millis() - tmrControl >= 100)
    {
        tmrControl = millis();
        static int lastVal = -1;
        // Если toggle == true, берем значение слайдера, иначе 0
        int currentVal = db[kk::toggle].toBool() ? db[kk::slider].toInt() : 0;
        currentVal = constrain(currentVal, 0, 1023);
        if (currentVal != lastVal)
        {
            analogWrite(D7, currentVal);
            lastVal = currentVal;
        }
    }
}
