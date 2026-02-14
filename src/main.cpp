
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
const DeviceAddress addr1 = {0x28, 0xB2, 0x54, 0x7F, 0x00, 0x00, 0x00, 0xCF};
const DeviceAddress addr2 = {0x28, 0x39, 0xE2, 0x6E, 0x01, 0x00, 0x00, 0x12};
// Глобальные переменные состояния
float temp1 = 0;
float temp2 = 0;
uint32_t tmr;
String longMessage = "";
int16_t textX = 128;

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
    // db.init(kk::pass, "some pass");
    // db.init(kk::uintw, 64u);
    // db.init(kk::intw, -10);
    // db.init(kk::int64w, 1234567ll);
    // db.init(kk::color, 0xff0000);
    db.init(kk::toggle, true);
    db.init(kk::slider, -3.5);
    // db.init(kk::selectw, (uint8_t)1);
    // db.init(kk::date, 1719941932);
    // db.init(kk::timew, 60);
    // db.init(kk::datime, 1719941932);
    // db.init(kk::sldmin, -5);
    // db.init(kk::sldmax, 5);
    setStampZone(3);

    // Проверяем, создана ли база
    if (!db.has(kk::logic))
    {
        db[kk::logic] = "{}";
        db.update();
    }

    // 3. Железо
    Wire.begin();
    // 1. Инициализация физического RTC
    rtc.begin();
    // Явно указываем класс для обхода ошибки ambiguous
    uint32_t unix = rtc.GyverDS3231Min::getUnix();
    // Установка системного времени ESP
    timeval tv = {(time_t)unix, 0};
    settimeofday(&tv, NULL);
    // Установка часового пояса (для 24-часового формата)
    setStampZone(3);
    // Синхронизируем программные часы SettingsGyver с системными
    // Просто присваиваем unix-штамп
    sett.rtc = unix;
    //Serial.print("RTC Unix: ");
    //Serial.println(unix);
    
    initSensors();
    // 4. Сеть
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // ... цикл подключения ...
    WiFi.softAP("AP ESP");
    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);
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

    // --- 1. Синхронизация времени (раз в час, а не каждую секунду) ---
    static uint32_t syncTmr;
    if (millis() - syncTmr >= 3600000)
    { // 1 час
        syncTmr = millis();
        if (sett.rtc.synced())
        {
            rtc.setUnix(sett.rtc.getUnix());
            Serial.println("RTC Synced with Web");
        }
    }

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
