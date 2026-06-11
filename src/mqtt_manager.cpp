#include "config.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

static uint32_t tmrMqttSend;
static uint32_t tmrMqttReconnect;

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
        db[kk::toggle] = (msg == "1" || msg == "true" || msg == "ON");
    if (strTopic == MQTT_TOPIC_SET_SELECT)
        db[kk::selectw] = msg.toInt();
}

// --- HA Discovery: публикуем конфигурацию для Home Assistant ---
static void publishDiscoverySensor(const char *sensorId, const char *name, const char *valTpl, const char *unit, const char *devClass)
{
    String topic = String(HA_DISCOVERY_PREFIX) + "/sensor/" + HA_DEVICE_ID + "_" + sensorId + "/config";
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

void sendMQTTDiscovery()
{
    if (!mqtt.connected()) return;

    publishDiscoverySensor("temp1", "Температура дом",
        "{{ value_json.temp1 }}", "°C", "temperature");
    publishDiscoverySensor("temp2", "Температура улица",
        "{{ value_json.temp2 }}", "°C", "temperature");

    // Реле (switch)
    {
        String topic = String(HA_DISCOVERY_PREFIX) + "/switch/" + HA_DEVICE_ID + "_relay/config";
        JsonDocument doc;
        doc["name"] = "Реле";
        doc["uniq_id"] = String(HA_DEVICE_ID) + "_relay";
        doc["stat_t"] = MQTT_TOPIC_STATE;
        doc["cmd_t"] = MQTT_TOPIC_SET_TOGGLE;
        doc["val_tpl"] = "{{ value_json.toggle }}";
        doc["payload_on"] = "1";
        doc["payload_off"] = "0";
        doc["state_on"] = true;
        doc["state_off"] = false;
        doc["dev"]["ids"] = HA_DEVICE_ID;
        String output;
        serializeJson(doc, output);
        mqtt.publish(topic.c_str(), output.c_str(), true);
    }

    // Мощность (number)
    {
        String topic = String(HA_DISCOVERY_PREFIX) + "/number/" + HA_DEVICE_ID + "_power/config";
        JsonDocument doc;
        doc["name"] = "Мощность";
        doc["uniq_id"] = String(HA_DEVICE_ID) + "_power";
        doc["stat_t"] = MQTT_TOPIC_STATE;
        doc["cmd_t"] = MQTT_TOPIC_SET_SLIDER;
        doc["val_tpl"] = "{{ value_json.slider }}";
        doc["min"] = 0;
        doc["max"] = 1023;
        doc["step"] = 1;
        doc["unit_of_meas"] = "PWM";
        doc["dev"]["ids"] = HA_DEVICE_ID;
        String output;
        serializeJson(doc, output);
        mqtt.publish(topic.c_str(), output.c_str(), true);
    }

    // Режим (select)
    {
        String topic = String(HA_DISCOVERY_PREFIX) + "/select/" + HA_DEVICE_ID + "_mode/config";
        JsonDocument doc;
        doc["name"] = "Режим";
        doc["uniq_id"] = String(HA_DEVICE_ID) + "_mode";
        doc["stat_t"] = MQTT_TOPIC_STATE;
        doc["cmd_t"] = MQTT_TOPIC_SET_SELECT;
        doc["val_tpl"] = "{{ value_json.select }}";
        doc["options"][0] = "var1";
        doc["options"][1] = "var2";
        doc["options"][2] = "hello";
        doc["dev"]["ids"] = HA_DEVICE_ID;
        String output;
        serializeJson(doc, output);
        mqtt.publish(topic.c_str(), output.c_str(), true);
    }

    Serial.println("HA Discovery sent");
}

// --- Публикация статуса ---
void sendMQTTStatus()
{
    if (!mqtt.connected()) return;

    JsonDocument doc;
    doc["temp1"] = db[kk::tmp1].toFloat();
    doc["temp2"] = db[kk::tmp2].toFloat();
    doc["slider"] = db[kk::slider].toFloat();
    doc["toggle"] = db[kk::toggle].toBool();
    doc["select"] = db[kk::selectw].toInt();
    doc["uptime"] = millis() / 1000;

    char buffer[256];
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
        sendMQTTDiscovery();
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