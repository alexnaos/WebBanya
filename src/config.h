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
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>

// --- Настройки дисплея ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

// --- Настройки датчиков и периферии ---
#define ONE_WIRE_BUS       D5
#define LED_PIN            D7
#define TEMP_UPDATE_MS     1000
#define OLED_UPDATE_MS     500
#define PWM_FREQ           10000
#define PWM_MAX            1023

// --- Сеть ---
#define WIFI_SSID_1 "Ban2"
#define WIFI_PASS_1 "2716192023"
#define WIFI_SSID_2 "Sloboda100"
#define WIFI_PASS_2 "2716192023"
#define WIFI_TIMEOUT_MS    15000

// --- MQTT ---
#define MQTT_SERVER "192.168.1.23"
#define MQTT_PORT   1883
#define MQTT_TOPIC_STATE   "esp/state"
#define MQTT_TOPIC_SET_SLIDER "esp/set/slider"
#define MQTT_TOPIC_SET_TOGGLE "esp/set/toggle"
#define MQTT_TOPIC_SET_SELECT "esp/set/select"
#define MQTT_SEND_MS       2000
#define MQTT_CLIENT_NAME   "ESP_Banya"

// --- Home Assistant Discovery ---
#define HA_DISCOVERY_PREFIX "homeassistant"
#define HA_DEVICE_NAME      "WebBanya3"
#define HA_DEVICE_ID        "webbanya3_esp"
#define MDNS_NAME           "webbanya3"

// --- Часовой пояс ---
#define TIMEZONE 3

// Ключи базы данных
enum kk : size_t {
    txt,
    toggle,
    slider,
    selectw,
    tmp1,
    tmp2,
    lbl1,
    lbl2,
    date,
    timew,
    datime,
    btn1,
    btn2,
};

// Адреса датчиков DS18B20
extern const DeviceAddress addr1;
extern const DeviceAddress addr2;

// Глобальные объекты
extern GyverDBFile db;
extern SettingsGyver sett;
extern sets::Logger logger;
extern Adafruit_SSD1306 display;
extern GyverDS3231 rtc;
extern ESP8266WiFiMulti wifiMulti;
extern WiFiClient espClient;
extern PubSubClient mqtt;

// Объявления функций
void initWiFi();
void initRTC();
void initSensors();
void initMQTT();
void initOTA();
void handleMQTT();
void handleSensors();
void handleControl();
void handleOTA();
void initDisplay();
void updateOLED();
void syncRTCFromDB();
void build(sets::Builder &b);
void update(sets::Updater &upd);
void sendMQTTDiscovery();

#endif