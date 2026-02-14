
#include <Arduino.h>
#include "config.h"
#include "ui_portal.h"

// Создание объектов
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
GyverDS3231 rtc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
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

void updateOLED()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // Просто создаем Datime из sett.rtc. Он сам подтянет локальное время
    Datime dt = sett.rtc;

    // Вывод ЧЧ:ММ:СС
    if (dt.hour < 10)
        display.print('0');
    display.print(dt.hour);
    display.print(':');
    if (dt.minute < 10)
        display.print('0');
    display.print(dt.minute);
    display.print(':');
    if (dt.second < 10)
        display.print('0');
    display.print(dt.second);

    // Мигающая звездочка
    if (millis() % 1000 < 500)
    {
        display.print("**");
    }

    // --- ПРАВЫЙ БЛОК (64-127 пикселей): ТЕКСТ / СТАТУС ---
    display.setCursor(64, 0);
    display.setTextSize(1);

    if (WiFi.status() == WL_CONNECTED)
    {
        display.print("ONLINE");
        // Если нужно вывести IP маленьким шрифтом под надписью, можно так:
        // display.setCursor(64, 8);
        // display.print(WiFi.localIP());
    }
    else
    {
        display.print("OFFLINE");
    }

    // --- 2. Статус и Выбор ---
    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print("S:");
    display.print(db[kk::toggle] ? "ON" : "OFF");
    display.setCursor(70, 12); // Немного сместим V, чтобы не слипалось
    display.print("V:");
    // Упрощаем логику вывода
    display.print(db[kk::selectw].toInt() + 1);
    // --- 3. Слайдер ---
    display.setCursor(0, 30);
    display.print("R:");
    display.print(db[kk::slider].toFloat(), 1);
    // --- 4. Температура (Берем готовые значения из переменных) ---
    display.setTextSize(2);
    // --- 4. Температура (Берем напрямую из БД) ---
    display.setTextSize(2);
    display.setCursor(0, 49);

    float t1 = db[kk::tmp1].toFloat();
    float t2 = db[kk::tmp2].toFloat();

    if (t1 <= -50 || t1 >= 125)
        display.print("--");
    else
        display.print(t1, 1);

    display.print(" ");

    if (t2 <= -50 || t2 >= 125)
        display.print("--");
    else
        display.print(t2, 1);

    display.display();
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
    // 1. Файловая система (LittleFS) — основа всего
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS Error!");
    }

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
   
    // 3. Железо
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("OLED Error"));
    }

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
    Serial.print("RTC Unix: ");
    Serial.println(unix);
    display.clearDisplay();    
    display.display();
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

    sett.tick(); // Важно для работы времени и веба

    // 1. Авто-синхронизация железного модуля из веба/NTP
    static bool timeIsSynced = false;
    if (sett.rtc.synced() && !timeIsSynced)
    {
        // Как только время в системе стало точным (из браузера/NTP)
        // Записываем его в физический модуль DS3231
        rtc.setUnix(sett.rtc.getUnix());
        timeIsSynced = true;
        Serial.println("RTC Hardware updated from Web!");
    }
    if (!sett.rtc.synced())
        timeIsSynced = false;

    // 2. Обновление OLED раз в секунду
    static uint32_t oledTmr;
    if (millis() - oledTmr >= 1000)
    {
        oledTmr = millis();
        updateOLED();
    }

    // --- Управление пином D7 (Совмещаем Toggle и Slider) ---
    static uint32_t tmrControl;
    if (millis() - tmrControl >= 100)
    {
        tmrControl = millis();

        static int lastVal = -1;
        int currentVal = 0;

        // Если выключатель ВКЛ, берем значение со слайдера
        if (db[kk::toggle].toBool())
        {
            currentVal = constrain(db[kk::slider].toInt(), 0, 1023);
        }
        else
        {
            currentVal = 0; // Если выключатель ВЫКЛ, гасим пин
        }

        // Обновляем пин только если значение реально изменилось
        if (currentVal != lastVal)
        {
            analogWrite(D7, currentVal);
            lastVal = currentVal;
            Serial.print("D7 PWM: ");
            Serial.println(currentVal);
        }
    }

    // --- ТАЙМЕР 1: ЭКРАН (100 мс) ---
    static uint32_t tmrOLED;
    if (millis() - tmrOLED >= 100)
    {
        tmrOLED = millis();
        updateOLED();
    }
handleSensors();  // Опрос датчиков
   
    sett.reload();
}