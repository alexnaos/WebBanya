#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverDS3231.h>
#include <ArduinoJson.h>
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
#define LED_PIN D7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// GyverDS3231 rtc;

GyverDS3231 ds; // Железный модуль (адрес 0x68)
StampKeeper sk; // Программные часы (из SettingsGyver)

uint32_t tmr;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Глобальные переменные для управления прокруткой
String longMessage = "";
int16_t textX = 128; // Начальная позиция
// Константы и настройки
#define WIFI_SSID "Sloboda100"
#define WIFI_PASS "2716192023"
#define TEMP_UPDATE_INTERVAL 1000 // 0.5 секунд
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Sloboda43 work!", &db);

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

        // // 3. Проверяем условия (Время + День недели + Температура)
        // bool timeOk = (sk.hour() >= hStart && sk.hour() < hEnd);
        // bool dayOk = (days.indexOf(String(rtc.weekDay())) != -1);
        // bool tempOk = (kk::tmp1 < tempSet); // temp1 мы берем из вашего loop
        //                                     // float t1 = db[kk::tmp1].toFloat();

        // // 4. Управляем реле
        // if (enabled && timeOk && dayOk && tempOk)
        // {
        //     digitalWrite(LED_PIN, HIGH); // Включить
        //     db[kk::toggle] = true;       // Обновить статус в интерфейсе
        // }
        // else
        // {
        //     digitalWrite(LED_PIN, LOW); // Выключить
        //     db[kk::toggle] = false;
        // }
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
    b.Time(kk::timew, "Время системы"); // Это заставит браузер синхро
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
    // Третий аргумент — это само значение, которое отобразится справа
    b.Label(kk::lbl2, "millis()", "", sets::Colors::Red);
    // sets::Group g(b, "Group 2"); // Убран комментарий, т.к. не используется
    b.Color(kk::color, "Цвет");
    b.Switch(kk::toggle, "Реле");
    b.Select(kk::selectw, "Выбор", "var1;var2;hello");
    // b.Slider(kk::slider, "Мощность", -10, 10, 0.5, "deg");
    // Правильный формат: слайдер(ID, имя, мин, макс, шаг)
    b.Slider(kk::slider, "Мощность", 0, 1023, 1);

    // Используйте указатель (&), чтобы библиотека сама писала значение в базу
    // b.Slider(&db[kk::slider], gh::Int).name("Мощность").range(0, 1023, 1);

    b.Slider2(kk::sldmin, kk::sldmax, "Установка", -10, 10, 0.5, "deg");
    // b.Log(logger);
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
        //b.Time(kk::timew, "Time");
        b.DateTime(kk::datime, "Datime");

        if (b.beginMenu("Submenu"))
        {
            if (b.beginGroup("Menu Switches")) // Переименовано для уникальности ID
            {
                b.Switch("sw1"_h, "switch 1");
                b.Switch("sw2"_h, "switch 2");
                b.Switch("sw3"_h, "switch 3");
                b.endGroup();
            }
            b.endMenu();
        }
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
                db.clear();
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
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Вывод времени
    display.setCursor(0, 0);
    if (sk.hour() < 10)
        display.print('0');
    display.print(sk.hour());
    display.print(':');
    if (sk.minute() < 10)
        display.print('0');
    display.print(sk.minute());

    // Мигающая звездочка в конце этой же строки
    if (sett.rtc.synced() && millis() % 1000 < 500)
    {
        display.print(" *");
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
    // Явно преобразуем значение из БД в float для апдейтера
    upd.update(kk::tmp1, db[kk::tmp1].toFloat(), 2);
    upd.update(kk::tmp2, db[kk::tmp2].toFloat(), 2);
}

void setup()
{

    // 1. Конфигурация пинов
    pinMode(D7, OUTPUT);
    analogWriteRange(1023);
    analogWriteFreq(20000);
    analogWrite(D7, 0);

    // 2. База данных и файлы (СИСТЕМА)
    if (!LittleFS.begin())
        Serial.println("FS Error");
    db.begin();
    // Инициализация ключей...
    db.init(kk::slider, 0.0f); // убедитесь, что тип данных совпадает (float)
    db.init(kk::txt, "text");
    db.init(kk::tmp1, 0.0f);
    db.init(kk::tmp2, 0.0f);

    if (!db.has(kk::logic))
    {
        db[kk::logic] = "{}";
        db.update();
    }

    loadLogicFromFile();

    // 3. Шина I2C и Железо
    Wire.begin();          // Сначала запускаем шину
    Wire.setClock(400000); // Ускоряем шину до 400кГц для OLED

    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        display.clearDisplay();
        display.display();
    }
    ds.begin(); // Инициализация железного модуля GyverDS3231 ds;

    // 1. Получаем структуру времени из модуля
    Datime dt = ds.getTime();

    sk = dt.getUnix();

    // Проверка в монитор порта
    Serial.print("Time Synced: ");
    Serial.print(sk.hour());
    Serial.print(":");
    Serial.println(sk.minute());

    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    // 4. Сеть (в самом конце)
    WiFi.mode(WIFI_STA); // Если не используете AP, лучше только STA
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint8_t tries = 20;
    while (WiFi.status() != WL_CONNECTED && tries--)
    {
        delay(250); // Уменьшил delay, чтобы не вешать стек
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());
    // 5. Запуск интерфейса
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);
    setStampZone(3);
}




void loop()
{
    sett.tick();
    sk.tick(); // Обязательно для хода времени
               // Синхронизация: каждые 60 секунд обновляем железный модуль из программного
    static uint32_t syncTmr;
    if (millis() - syncTmr >= 60000)
    {
        syncTmr = millis();
        // 1. Получаем Unix из программных часов методом getUnix()
        uint32_t currentUnix = sk.getUnix();

        // 2. Создаем структуру Datime из этого значения
        Datime dt(currentUnix);

        ds.setTime(dt);
    }

    // --- Слайдер ---
    static uint32_t tmrSlider;
    if (millis() - tmrSlider >= 100)
    {
        tmrSlider = millis();
        int pwmValue = constrain((int)db[kk::slider].toFloat(), 0, 1023);
        static int lastPwmValue = -1;
        if (pwmValue != lastPwmValue)
        {
            analogWrite(D7, pwmValue);
            lastPwmValue = pwmValue;
        }
    }

    // --- Дисплей ---
    static uint32_t tmrOLED;
    if (millis() - tmrOLED >= 500)
    {
        tmrOLED = millis();
        updateOLED();
    }

    // --- Датчики ---
    static uint32_t tmrSensors;
    if (millis() - tmrSensors >= 2000)
    {
        tmrSensors = millis();

        float t1 = sensors.getTempC(addr1);
        float t2 = sensors.getTempC(addr2);

        // Записываем с округлением до 2 знаков
        if (t1 > -50 && t1 < 125)
            db[kk::tmp1] = round(t1 * 100.0f) / 100.0f;
        else
            db[kk::tmp1] = 0.0f;

        if (t2 > -50 && t2 < 125)
            db[kk::tmp2] = round(t2 * 100.0f) / 100.0f;
        else
            db[kk::tmp2] = 0.0f;

        sensors.requestTemperatures();

        // sett.reload();
    }
}
