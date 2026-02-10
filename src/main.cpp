 
#include <Arduino.h>
#include "config/config.h"
#include "modules/ui.h"
// ФИЗИЧЕСКОЕ СОЗДАНИЕ ОБЪЕКТА (ОПРЕДЕЛЕНИЕ)
// Имя "sett" должно совпадать с тем, что вы написали в config.h после extern
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Sloboda43", &db);

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
    // Третий аргумент — это само значение, которое отобразится справа
    b.Label(kk::lbl2, "millis()", "", sets::Colors::Red);
    // sets::Group g(b, "Group 2"); // Убран комментарий, т.к. не используется
    b.Color(kk::color, "Цвет");
    b.Switch(kk::toggle, "Реле");
    b.Select(kk::selectw, "Выбор", "var1;var2;hello");
    b.Slider(kk::slider, "Мощность", -10, 10, 0.5, "deg");
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
        b.Time(kk::timew, "Time");
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
    display.print(db[kk::color].toString());
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
    display.setCursor(0, 49);
    if (temp1 <= -100)
        display.print("--"); // Проверка на ошибку
    else
        display.print(temp1, 1);
    display.print(" ");
    if (temp2 <= -100)
        display.print("--");
    else
        display.print(temp2, 1);
    display.display();
}

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
