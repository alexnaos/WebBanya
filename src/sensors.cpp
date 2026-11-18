#include "config.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Автоопределённые датчики
DeviceAddress sensorAddrs[MAX_SENSORS];
float         sensorTemps[MAX_SENSORS];
uint8_t       sensorCount = 0;

static uint32_t tmrTemp;
static uint32_t tmrRescan;
static uint8_t  lastCount = 0;

// Печать адреса датчика в Serial (для логов)
static void printSensorAddress(const DeviceAddress addr)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (addr[i] < 16) Serial.print('0');
        Serial.print(addr[i], HEX);
    }
}

// Сканирование шины OneWire и сохранение найденных датчиков
void scanSensors()
{
    // begin() переищет устройства на шине и обновит внутренний счётчик
    sensors.begin();
    sensors.setWaitForConversion(false);

    uint8_t count = sensors.getDeviceCount();
    if (count > MAX_SENSORS) count = MAX_SENSORS;

    uint8_t newCount = 0;
    for (uint8_t i = 0; i < count; i++)
    {
        DeviceAddress addr;
        if (sensors.getAddress(addr, i))
        {
            memcpy(sensorAddrs[newCount], addr, 8);
            sensorTemps[newCount] = DEVICE_DISCONNECTED_C;
            Serial.print("Sensor #");
            Serial.print(newCount);
            Serial.print(" found: 28-");
            printSensorAddress(addr);
            Serial.println();
            newCount++;
        }
    }
    sensorCount = newCount;
    Serial.print("DS18B20 found: ");
    Serial.println(sensorCount);

    sensors.requestTemperatures();

    // Если количество датчиков изменилось - обновить HA Discovery
    if (sensorCount != lastCount)
    {
        lastCount = sensorCount;
        sendMQTTDiscovery();
    }
}

void initSensors()
{
    scanSensors();
}

void handleSensors()
{
    // Периодический поиск новых датчиков на шине
    if (millis() - tmrRescan >= TEMP_RESCAN_MS)
    {
        tmrRescan = millis();
        scanSensors();
    }

    if (millis() - tmrTemp >= TEMP_UPDATE_MS)
    {
        tmrTemp = millis();

        // Читаем температуры всех найденных датчиков
        for (uint8_t i = 0; i < sensorCount; i++)
        {
            float t = sensors.getTempC(sensorAddrs[i]);
            if (t != DEVICE_DISCONNECTED_C)
            {
                sensorTemps[i] = t;
                db[kk::sensorKeyBase + i] = t;   // для авто-обновления в вебморде
            }
            else
            {
                sensorTemps[i] = DEVICE_DISCONNECTED_C;
            }
        }

        sensors.requestTemperatures();
    }
}