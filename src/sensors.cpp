#include "config.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const DeviceAddress addr1 = {0x28, 0xB2, 0x54, 0x7F, 0x00, 0x00, 0x00, 0xCF};
const DeviceAddress addr2 = {0x28, 0x39, 0xE2, 0x6E, 0x01, 0x00, 0x00, 0x12};

static uint32_t tmrTemp;
float temp1 = 0;
float temp2 = 0;

void initSensors()
{
    sensors.begin();
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
}

void handleSensors()
{
    if (millis() - tmrTemp >= TEMP_UPDATE_MS)
    {
        tmrTemp = millis();

        float t1 = sensors.getTempC(addr1);
        float t2 = sensors.getTempC(addr2);

        if (t1 != DEVICE_DISCONNECTED_C)
        {
            temp1 = t1;
            db[kk::tmp1] = temp1;
        }
        if (t2 != DEVICE_DISCONNECTED_C)
        {
            temp2 = t2;
            db[kk::tmp2] = temp2;
        }

        sensors.requestTemperatures();
    }
}