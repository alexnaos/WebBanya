#ifndef CONFIG_H
#define CONFIG_H
#define LED_PIN D7
#define DISPLAY_VCC_PIN D6 // Пин питания дисплея
#include <Arduino.h>
#include <GyverDS3231.h>
#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

// Константы и настройки
#define WIFI_SSID "Sloboda100"
#define WIFI_PASS "2716192023"
#define TEMP_UPDATE_INTERVAL 1000 // 0.5 секунд
#define LED_PIN D7
#define ONE_WIRE_BUS D5 // Пин D1 на Wemos D1 Mini (GPIO5)


// Объявления глобальных объектов
extern GyverDS3231 rtc;
extern GyverDBFile db;
extern SettingsGyver sett;
extern Adafruit_SSD1306 display;
extern DeviceAddress addr1, addr2; // Если используете датчики
extern OneWire oneWire(ONE_WIRE_BUS);
extern DallasTemperature sensors(&oneWire);
extern GyverDS3231 rtc;

extern float temp1;
extern float temp2;


// Enum для ключей базы данных
namespace kk {
    // Переменные
enum kk : size_t
{
    txt,    pass,    uintw,    intw,    int64w,
    color,    toggle,    slider,    selectw,
    sldmin,    sldmax,    lbl1,    lbl2,
    date,    timew,    datime,    btn1,
    btn2,    tmp1,    tmp2,    logic,
};


}

// Прототипы функций, которые будут использоваться в main.cpp
void build(sets::Builder& b);
void updateOLED();
void runAutomation();
void loadLogicFromFile();


#endif
