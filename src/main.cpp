#include <Arduino.h>
#include "config.h"
#include "ui_portal.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;


// Создание объектов
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
GyverDS3231 rtc;
GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("Slobanya3 разделен!", &db);
sets::Logger logger(200);
#include <PubSubClient.h> // Не забудь установить!
#include <ArduinoJson.h>  // Для удобной передачи пачкой
WiFiClient espClient;
PubSubClient mqtt(espClient);

// --- Функция приема команд из Node-RED ---
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (int i = 0; i < length; i++)
        msg += (char)payload[i];

    String strTopic = String(topic);

    if (strTopic == TOPIC_SET_SLIDER)
        db[kk::slider] = msg.toFloat();
    if (strTopic == TOPIC_SET_TOGGLE)
        db[kk::toggle] = (msg == "1" || msg == "true");
    if (strTopic == TOPIC_SET_SELECT)
        db[kk::selectw] = msg.toInt();

    //db.update(); // Чтобы изменения сразу ушли в UI и на OLED
}

void sendMqttStatus()
{
    if (!mqtt.connected())
        return;

    // Собираем JSON, чтобы не слать 10 разных топиков
    StaticJsonDocument<200> doc;
    doc["temp1"] = db[kk::tmp1].toFloat();
    doc["temp2"] = db[kk::tmp2].toFloat();
    doc["slider"] = db[kk::slider].toFloat();
    doc["toggle"] = db[kk::toggle].toBool();
    doc["select"] = db[kk::selectw].toInt();
    doc["uptime"] = millis() / 1000;
    char buffer[200];
    serializeJson(doc, buffer);
    mqtt.publish(TOPIC_STATE, buffer);
}

void mqttReconnect()
{
    if (millis() % 5000 != 0)
        return; // Пытаемся раз в 5 сек без блокировки loop
    if (!mqtt.connected())
    {
        if (mqtt.connect("ESP_Main_Module"))
        {
            mqtt.subscribe(TOPIC_SET_SLIDER);
            mqtt.subscribe(TOPIC_SET_TOGGLE);
            mqtt.subscribe(TOPIC_SET_SELECT);
        }
    }
}

void initTime()
{
    rtc.begin();
    uint32_t unix = rtc.GyverDS3231Min::getUnix();
    // Синхронизируем всё при старте
    timeval tv = {(time_t)unix, 0};
    settimeofday(&tv, NULL);
    setStampZone(3);
    sett.rtc = unix;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    // 1. Конфигурация пинов
    pinMode(D7, OUTPUT);
    analogWriteRange(1023);
    analogWriteFreq(10000);
    analogWrite(D7, 0);
    Serial.println("LED is ON");
    LittleFS.begin();
    db.begin();
    // Инициализируем базу (только если ключей еще нет)
    db.init(kk::txt, "text");
    db.init(kk::tmp1, 0.0f);
    db.init(kk::tmp2, 0.0f);
    db.init(kk::toggle, true);
    db.init(kk::slider, -3.5);
    setStampZone(3);
    // Проверяем, создана ли база
    if (!db.has(kk::logic))
    {
        db[kk::logic] = "{}";
        db.update();
    }
    // 3. Железо
    Wire.begin();
    initTime(); // Теперь setup выглядит аккуратно
    initSensors();

  // 1. Устанавливаем режим
WiFi.mode(WIFI_STA);

// 2. Регистрируем все доступные точки доступа
wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

Serial.println("WiFi: Connecting...");

// 3. Ждем первого подключения (необязательно, но полезно для MQTT/Времени)
// run() вернет WL_CONNECTED, когда выберет лучшую сеть
while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}

Serial.println("\nWiFi Connected to: " + WiFi.SSID());


    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.config.theme = sets::Colors::Green;
    setStampZone(3); // Часовой пояс
    sett.onBuild(build);
    sett.onUpdate(update);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
}

void loop() {
    // 1. Поддержка Wi-Fi (автоматическое переключение на лучший сигнал)
    // run() вызывается постоянно, он сам решит, нужно ли переподключаться
    wifiMulti.run(); 

    // 2. Системные задачи (должны работать ВСЕГДА)
    sett.tick();     // Обслуживание веб-интерфейса
    db.tick();       // Сохранение настроек в базу
    handleSensors(); // Опрос датчиков

    // 3. MQTT логика (только если есть Wi-Fi)
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqtt.connected()) {
            mqttReconnect();
        } else {
            mqtt.loop();
            // Отправка данных каждые 2 секунды
            static uint32_t tmr_mqtt_send;
            if (millis() - tmr_mqtt_send >= 2000) {
                tmr_mqtt_send = millis();
                sendMqttStatus();
            }
        }
    }

    // 4. Управление нагрузкой (ШИМ на D7) — работает автономно
    static uint32_t tmrControl;
    if (millis() - tmrControl >= 100) {
        tmrControl = millis();
        static int lastVal = -1;
        
        // Берем значения из базы данных GyverDB
        int currentVal = db[kk::toggle].toBool() ? db[kk::slider].toInt() : 0;
        currentVal = constrain(currentVal, 0, 1023);
        
        if (currentVal != lastVal) {
            analogWrite(D7, currentVal);
            lastVal = currentVal;
        }
    }
}
