#include "config.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

static uint32_t tmrMqttSend;
static uint32_t tmrMqttReconnect;
static const char* selectOptions[] = {"var1", "var2", "hello"};
static const int selectOptionCount = 3;

// --- Callback приёма команд из MQTT ---
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (unsigned int i = 0; i < length; i++)
        msg += (char)payload[i];

    String strTopic = String(topic);

    if (strTopic == MQTT_TOPIC_SET_SLIDER)
        db[kk::slider] = msg.toFloat();

    if (strTopic == MQTT_TOPIC_SET_TOGGLE)
    {
        db[kk::toggle] = (msg == "ON" || msg == "1" || msg == "true");
        Serial.print("Toggle set to: ");
        Serial.println(db[kk::toggle].toBool() ? "ON" : "OFF");
    }

    if (strTopic == MQTT_TOPIC_SET_SELECT)
    {
        // HA может прислать как число (индекс), так и строку
        int idx = msg.toInt();
        if (idx >= 0 && idx < selectOptionCount)
        {
            db[kk::selectw] = idx;
        }
        else
        {
            // Поиск по имени опции
            for (int i = 0; i < selectOptionCount; i++)
            {
                if (msg == selectOptions[i])
                {
                    db[kk::selectw] = i;
                    break;
                }
            }
        }
        Serial.print("Select set to: ");
        Serial.println(db[kk::selectw].toInt());
    }
}

// --- HA Discovery: публикуем конфигурацию для Home Assistant ---
#if ENABLE_HA_DISCOVERY
static void publishDiscoverySensor(const char *sensorId, const char *name,
    const char *valTpl, const char *unit, const char *devClass)
{
    String topic = String(HA_DISCOVERY_PREFIX) + "/sensor/"
        + HA_DEVICE_ID + "_" + sensorId + "/config";
    JsonDocument doc;
    doc["name"] = name;
    doc["uniq_id"] = String(HA_DEVICE_ID) + "_" + sensorId;
    doc["stat_t"] = MQTT_TOPIC_STATE;
    doc["val_tpl"] = valTpl;
    doc["unit_of_meas"] = unit;
    if (devClass) doc["dev_cla"] = devClass;
    doc["dev"]["ids"] = HA_DEVICE_ID;
    doc["dev"]["name"] = HA_DEVICE_NAME;
    doc["dev"]["mf"] = "ESP8266";
    doc["dev"]["sw"] = "1.0";
    String output;
    serializeJson(doc, output);
    mqtt.publish(topic.c_str(), output.c_str(), true);
}
#endif

void sendMQTTDiscovery()
{
    // HA Discovery disabled - no-op
    (void)mqtt;
}
// --- Публикация статуса ---
void sendMQTTStatus()
{
    if (!mqtt.connected()) return;

    JsonDocument doc;

    // Температуры всех найденных датчиков: temp1, temp2, ... tempN
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        float temp = sensorTemps[i];
        // Проверяем на валидность значения: не NAN, не DEVICE_DISCONNECTED_C
        if (!isnan(temp) && temp != DEVICE_DISCONNECTED_C)
        {
            String key = "temp" + String(i + 1);
            doc[key.c_str()] = serialized(String(temp, 1));
        }
        // else: не добавляем ключ temp1/temp2, если значение невалидно
    }
    doc["SensorCount"] = sensorCount;
    doc["slider"] = db[kk::slider].toInt();

    // Switch: ON/OFF строкой, а не true/false
    doc["toggle"] = db[kk::toggle].toBool() ? "ON" : "OFF";

    // Select: слать имя опции, а не индекс
    int selIdx = db[kk::selectw].toInt();
    if (selIdx >= 0 && selIdx < selectOptionCount)
        doc["select_mode"] = selectOptions[selIdx];
    else
        doc["select_mode"] = "unknown";

    doc["select"] = selIdx; // оставим для обратной совместимости
    doc["uptime"] = millis() / 1000;

    char buffer[512];
    serializeJson(doc, buffer);
    mqtt.publish(MQTT_TOPIC_STATE, buffer);
}

// --- Переподключение MQTT ---
void mqttReconnect()
{
    if (mqtt.connected()) return;
    if (millis() - tmrMqttReconnect < 5000) return;
    tmrMqttReconnect = millis();

    Serial.println("MQTT: Connecting...");
    if (mqtt.connect(MQTT_CLIENT_NAME))
    {
        Serial.println("MQTT: Connected");
        mqtt.subscribe(MQTT_TOPIC_SET_SLIDER);
        mqtt.subscribe(MQTT_TOPIC_SET_TOGGLE);
        mqtt.subscribe(MQTT_TOPIC_SET_SELECT);
#if ENABLE_HA_DISCOVERY
        sendMQTTDiscovery();
#endif
    }
    else
    {
        Serial.print("MQTT: Failed, rc=");
        Serial.println(mqtt.state());
    }
}

// --- Инициализация MQTT ---
void initMQTT()
{
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
}

// --- Обработка MQTT в loop ---
void handleMQTT()
{
    if (!WiFi.isConnected()) return;

    if (!mqtt.connected())
    {
        mqttReconnect();
    }
    else
    {
        mqtt.loop();

        if (millis() - tmrMqttSend >= MQTT_SEND_MS)
        {
            tmrMqttSend = millis();
            sendMQTTStatus();
        }
    }
}