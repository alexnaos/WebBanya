#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <GyverDS3231.h>
#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

// Объявления глобальных объектов
extern GyverDS3231 rtc;
extern GyverDBFile db;
extern SettingsGyver sett;
extern Adafruit_SSD1306 display;
extern DallasTemperature sensors;
extern DeviceAddress addr1, addr2; // Если используете датчики

// Enum для ключей базы данных
namespace kk {
    enum {
        txt, tmp1, tmp2, toggle, selectw, slider, sldmin, sldmax,
        date, timew, datime, btn1, btn2, lbl2, color, logic
    };
}

// Прототипы функций, которые будут использоваться в main.cpp
void build(sets::Builder& b);
void updateOLED();
void runAutomation();
void loadLogicFromFile();

#endif
