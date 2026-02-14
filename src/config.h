#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverDS3231.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// --- Настройки дисплея ---
#define DISPLAY_VCC_PIN D6
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
// --- Настройки датчиков и периферии ---
#define ONE_WIRE_BUS D5
#define LED_PIN D7
#define TEMP_UPDATE_INTERVAL 1000
// --- Сеть ---
#define WIFI_SSID "Sloboda100"
#define WIFI_PASS "2716192023"

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

// ОБЪЯВЛЕНИЯ ОБЪЕКТОВ (через extern)
// Это говорит компилятору: "Объект существует, но создан он в другом месте"
extern GyverDBFile db;
extern SettingsGyver sett;
extern sets::Logger logger;
extern float temp1;
extern float temp2;

void initSensors();
void handleSensors();
void updateOLED();
void setup();
void loop();

#endif
