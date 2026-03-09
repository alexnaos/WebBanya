
#include <Arduino.h>
#include "config.h"
#include "ui_portal.h"
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

    db.update(); // Чтобы изменения сразу ушли в UI и на OLED
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
    analogWriteFreq(20000);
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
    // 4. Сеть
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // ... цикл подключения ...
    WiFi.softAP("AP ESP");
    // 5. Интерфейс (ПОСЛЕ БД)
    sett.begin();
    sett.config.theme = sets::Colors::Green;
    setStampZone(3); // Часовой пояс
    sett.onBuild(build);
    sett.onUpdate(update);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
}
void loop()
{
    sett.tick();     // Обслуживание веб-интерфейса (ОБЯЗАТЕЛЬНО)
    db.tick();       // Обслуживание базы данных (сохранение на диск)
    handleSensors(); // Опрос датчиков (внутри функции свой таймер)

    // MQTT логика
    if (!mqtt.connected())
    {
        mqttReconnect();
    }
    else
    {
        mqtt.loop();

        // Отправка данных по таймеру (например, каждые 2 секунды)
        static uint32_t tmr_mqtt_send;
        if (millis() - tmr_mqtt_send >= 2000)
        {
            tmr_mqtt_send = millis();
            sendMqttStatus();
        }
    }

    // --- 2. Управление нагрузкой (ШИМ на D7) ---
    static uint32_t tmrControl;
    if (millis() - tmrControl >= 100)
    {
        tmrControl = millis();
        static int lastVal = -1;
        // Если toggle == true, берем значение слайдера, иначе 0
        int currentVal = db[kk::toggle].toBool() ? db[kk::slider].toInt() : 0;
        currentVal = constrain(currentVal, 0, 1023);
        if (currentVal != lastVal)
        {
            analogWrite(D7, currentVal);
            lastVal = currentVal;
        }
    }
}
