
#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverDS3231.h>
#include <ArduinoJson.h>
#define DISPLAY_VCC_PIN D6 // Пин питания дисплея
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Зависит от платы, обычно -1
#include <OneWire.h>
#include <DallasTemperature.h>

// 2. Ваши проверенные адреса
DeviceAddress addr1 = {0x28, 0xB2, 0x54, 0x7F, 0x00, 0x00, 0x00, 0xCF};
DeviceAddress addr2 = {0x28, 0x39, 0xE2, 0x6E, 0x01, 0x00, 0x00, 0x12};
// 1. Настройка пина и библиотек
#define ONE_WIRE_BUS D5 // Пин D1 на Wemos D1 Mini (GPIO5)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
GyverDS3231 rtc;
// 3. Глобальные переменные для хранения температуры
float temp1 = 0;
float temp2 = 0;
uint32_t tmr;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Глобальные переменные для управления прокруткой
String longMessage = "";
int16_t textX = 128; // Начальная позиция
// Константы и настройки
#define WIFI_SSID "Sloboda100"
#define WIFI_PASS "2716192023"
#define TEMP_UPDATE_INTERVAL 1000 // 0.5 секунд
#define LED_PIN D7
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Slobanya2!", &db);

// Переменные
enum kk : size_t
{
    txt,
    pass,
    uintw,
    intw,
    int64w,
    color,
    toggle,
    slider,
    selectw,
    sldmin,
    sldmax,
    lbl1,
    lbl2,
    date,
    timew,
    datime,
    btn1,
    btn2,
    tmp1,
    tmp2,
    logic,
};

sets::Logger logger(200);

void runAutomation()
{
    String jsonStr = db[kk::logic].toString();
    JsonDocument doc;
    if (deserializeJson(doc, jsonStr) == DeserializationError::Ok)
    {

        // 2. Считываем правила из JSON
        bool enabled = doc["enabled"] | false;
        float tempSet = doc["temp_set"] | 20.0;
        int hStart = doc["h_start"] | 0;
        int hEnd = doc["h_end"] | 24;
        String days = doc["days"] | "1,2,3,4,5,6,7";

        // 3. Проверяем условия (Время + День недели + Температура)
        bool timeOk = (rtc.hour() >= hStart && rtc.hour() < hEnd);
        bool dayOk = (days.indexOf(String(rtc.weekDay())) != -1);
        bool tempOk = (temp1 < tempSet); // temp1 мы берем из вашего loop

        // 4. Управляем реле
        if (enabled && timeOk && dayOk && tempOk)
        {
            digitalWrite(LED_PIN, HIGH); // Включить
            db[kk::toggle] = true;       // Обновить статус в интерфейсе
        }
        else
        {
            digitalWrite(LED_PIN, LOW); // Выключить
            db[kk::toggle] = false;
        }
    }
}

void loadLogicFromFile()
{
    if (LittleFS.exists("/logic.json"))
    {
        File f = LittleFS.open("/logic.json", "r");
        if (f)
        {
            String content = f.readString();
            f.close();

            JsonDocument doc;
            if (!deserializeJson(doc, content))
            {
                db[kk::logic] = content;
                db.update(); // Для GyverDBFile сохраняем изменения
                Serial.println("Logic updated from /logic.json");
                runAutomation();
            }
        }
    }
    else
    {
        Serial.println("File /logic.json not found!");
    }
}

void build(sets::Builder &b)
{
    if (b.build.isAction())
    {
        logger.print("Set: 0x");
        logger.println(b.build.id, HEX);
    }

    if (b.beginGroup("Group 1")) // Группа 1
    {
        b.Input(kk::txt, "Text");
        // b.Pass(kk::pass, "Password");
        b.Label(kk::tmp1, "Дом");
        b.Label(kk::tmp2, "Улица");
        b.endGroup();
    }
    b.Switch(kk::toggle, "Реле");
    b.Select(kk::selectw, "Выбор", "var1;var2;hello");
    // b.Slider(kk::slider, "Мощность", -10, 10, 0.5, "deg");
    b.Slider(kk::slider, "Мощность", 0, 1023, 1);

    if (b.beginRow())
    {
        if (b.Button("click"))
        {
            Serial.println("click: " + String(b.build.pressed()));
        }
        if (b.ButtonHold("hold"))
        {
            Serial.println("hold: " + String(b.build.pressed()));
        }
        b.endRow();
    }

    if (b.beginGroup("Group3")) // Группа с датами и временем
    {
        b.Date(kk::date, "Date");
        b.Time(kk::timew, "Time");
        b.DateTime(kk::datime, "Datime");
        b.endGroup(); // Закрываем Group3
    }

    // --- Блок системных кнопок ---
    if (b.beginGroup("Система управления")) // Переименовано для уникальности ID
    {
        if (b.beginButtons())
        {
            if (b.Button(kk::btn1, "reload"))
            {
                Serial.println("reload");
                b.reload();
            }
            if (b.Button(kk::btn2, "clear db", sets::Colors::Blue))
            {
                Serial.println("clear db");
                // db.clear();
                db.update();
            }
            b.endButtons();
        }
        b.endGroup(); // Закрываем Систему управления
    }

    // ТЕПЕРЬ СЦЕНАРИЙ (вызывается на «чистом» уровне)
    if (b.beginGroup("Сценарий из файла"))
    {
        // 1. Читаем файл (убедитесь, что LittleFS.begin() в setup)
        if (LittleFS.exists("/logic.json"))
        {
            File f = LittleFS.open("/logic.json", "r");
            db[kk::logic] = f.readString(); // Записываем содержимое файла в ключ базы
            f.close();
        }

        // 2. Выводим Input
        b.Input(kk::logic, "JSON код файла");

        // 3. Кнопка
        if (b.Button("ВЫПОЛНИТЬ СЦЕНАРИЙ", sets::Colors::Green))
        {
            runAutomation();
            Serial.println("Manual run executed");
        }

        b.endGroup(); // Закрываем Сценарий из файла
    }
}

void updateOLED()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    // --- ЛЕВЫЙ БЛОК (0-63 пикселя): ВРЕМЯ ---
    display.setCursor(0, 0);
    // Часы
    if (rtc.hour() < 10)
        display.print('0');
    display.print(rtc.hour());
    display.print(':');

    // Минуты
    if (rtc.minute() < 10)
        display.print('0');
    display.print(rtc.minute());
    display.print(':');

    // Секунды
    if (rtc.second() < 10)
        display.print('0');
    display.print(rtc.second());
    // Мигающая звездочка синхронизации
    if (sett.rtc.synced() && millis() % 1000 < 500)
    {
        display.print(" *");
    }
    // --- ПРАВЫЙ БЛОК (64-127 пикселей): ТЕКСТ ---
    display.setCursor(64, 0);
    // toString() может быть длинным, библиотека сама перенесет его,
    // если он не влезет в остаток строки
    // display.print(db[kk::txt].toString());
    display.print(WiFi.localIP());

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

void update(sets::Updater &upd)
{
    upd.update(kk::lbl1, random(100));
    upd.update(kk::lbl2, millis());
    // upd.update(kk::tmp1, temp1);
    // upd.update(kk::tmp2, temp2);
    upd.update(kk::tmp1, db[kk::tmp1].toFloat(), 2);
    upd.update(kk::tmp2, db[kk::tmp2].toFloat(), 2);
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

    // --- Датчики (раз в 5 секунд - для температуры чаще не нужно) ---
    static uint32_t tmrSensors;
    if (millis() - tmrSensors >= 5000)
    {
        tmrSensors = millis();

        float t1 = sensors.getTempC(addr1);
        float t2 = sensors.getTempC(addr2);

        if (t1 > -50 && t1 < 125)
            db[kk::tmp1] = round(t1 * 10.0f) / 10.0f; // 1 знак достаточно
        else
            db[kk::tmp1] = 0.0f;

        if (t2 > -50 && t2 < 125)
            db[kk::tmp2] = round(t2 * 10.0f) / 10.0f;
        else
            db[kk::tmp2] = 0.0f;

        sensors.requestTemperatures();
    }
    sett.reload();
}