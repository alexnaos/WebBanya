#include <Arduino.h>
#include <LittleFS.h>
#include "config/config.h" // Подключаем общие объявления
#include "modules/ui.h"

// Объявление глобальных объектов здесь
GyverDS3231 rtc;
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Sloboda 43", &db);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
GyverDS3231 rtc;

float temp1 = 0;
float temp2 = 0;
uint32_t tmr;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Глобальные переменные для управления прокруткой
String longMessage = "";
int16_t textX = 128; // Начальная позиция

void update(sets::Updater &upd)
{
    upd.update(kk::lbl1, random(100));
    upd.update(kk::lbl2, millis());
    upd.update(kk::tmp1, temp1);
    upd.update(kk::tmp2, temp2);
}

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

    // Проверяем, создана ли база
    if (!db.has(kk::logic))
    {
        db[kk::logic] = "{}";
        db.update();
    }
    loadLogicFromFile();

    // 3. Железо
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("OLED Error"));
    }
    rtc.begin();
    if (rtc.isOK())
    {
        // Прямое приведение объекта rtc к типу Datime
        // Библиотека сама сконвертирует время в нужный формат без вызова getUnix()
        Datime dt = rtc;
        sett.rtc = dt.getUnix();
        Serial.println("RTC OK: Time loaded");
    }
    else
    {
        Serial.println("RTC NOT FOUND (OK: 0)");
    }

    display.clearDisplay();
    display.display();
    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();

    // 4. Сеть
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // ... цикл подключения ...
    WiFi.softAP("AP ESP");

    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);
    // sets::Code::update();
    sett.config.theme = sets::Colors::Green;

    setStampZone(3); // Часовой пояс
}

void loop()
{
    sett.tick(); // Обработка веб-интерфейса
    static bool timeIsSet = false;
    if (sett.rtc.synced() && !timeIsSet)
    {
        // Синхронизируем железный модуль из браузера
        rtc.setUnix(sett.rtc.getUnix());
        timeIsSet = true;
        Serial.println("Synced!");
    }
    if (!sett.rtc.synced())
        timeIsSet = false;
    // --- ВАШ ОСТАЛЬНОЙ КОД ---
    digitalWrite(LED_PIN, db[kk::toggle].toBool());

    // --- ТАЙМЕР 1: ЭКРАН (100 мс) ---
    static uint32_t tmrOLED;
    if (millis() - tmrOLED >= 100)
    {
        tmrOLED = millis();
        updateOLED();
    }

    // --- ТАЙМЕР 2: ДАТЧИКИ (2000 мс) ---
    static uint32_t tmrTemp;
    if (millis() - tmrTemp >= 2000)
    {
        tmrTemp = millis();
        float t1 = sensors.getTempC(addr1);
        float t2 = sensors.getTempC(addr2);

        if (t1 != DEVICE_DISCONNECTED_C)
        {
            temp1 = t1;
            db[kk::tmp1] = temp1;
        }
        if (t2 != DEVICE_DISCONNECTED_C)
        {
            temp2 = t2;
            db[kk::tmp2] = temp2;
        }
        sensors.requestTemperatures();
    }
}
