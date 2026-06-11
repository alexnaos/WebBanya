#include "config.h"

GyverDS3231 rtc;

void initRTC()
{
    Wire.begin();
    rtc.begin();

    if (rtc.isOK())
    {
        Datime dt = rtc;
        sett.rtc = dt.getUnix();
        Serial.println("RTC OK: Time loaded from DS3231");
    }
    else
    {
        Serial.println("RTC NOT FOUND");
    }

    setStampZone(TIMEZONE);
}

void syncRTCFromDB()
{
    uint32_t dbUnix = db[kk::date].toInt32();
    uint32_t sysUnix = (uint32_t)time(NULL);

    if (abs((long)dbUnix - (long)sysUnix) > 5)
    {
        rtc.setUnix(dbUnix);
        timeval tv = {(time_t)dbUnix, 0};
        settimeofday(&tv, NULL);
        Serial.println("RTC: Sync from DB done");
    }
}