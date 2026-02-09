#include "../config/config.h"
#include "automation.h"
#include "ui.h"


void runAutomation()
{
    String jsonStr = db[kk::logic].toString();
    JsonDocument doc;
    if (deserializeJson(doc, jsonStr) == DeserializationError::Ok)
    {

        // 2. Считываем правила из JSON
        bool enabled = doc["enabled"] | false;
        float tempSet = doc["temp_set"] | 20.0;
        int hStart = doc["h_start"] | 0;
        int hEnd = doc["h_end"] | 24;
        String days = doc["days"] | "1,2,3,4,5,6,7";

        // 3. Проверяем условия (Время + День недели + Температура)
        bool timeOk = (rtc.hour() >= hStart && rtc.hour() < hEnd);
        bool dayOk = (days.indexOf(String(rtc.weekDay())) != -1);
        bool tempOk = (temp1 < tempSet); // temp1 мы берем из вашего loop

        // 4. Управляем реле
        if (enabled && timeOk && dayOk && tempOk)
        {
            digitalWrite(LED_PIN, HIGH); // Включить
            db[kk::toggle] = true;       // Обновить статус в интерфейсе
        }
        else
        {
            digitalWrite(LED_PIN, LOW); // Выключить
            db[kk::toggle] = false;
        }
    }
}


void loadLogicFromFile()
{
    if (LittleFS.exists("/logic.json"))
    {
        File f = LittleFS.open("/logic.json", "r");
        if (f)
        {
            String content = f.readString();
            f.close();

            JsonDocument doc;
            if (!deserializeJson(doc, content))
            {
                db[kk::logic] = content;
                db.update(); // Для GyverDBFile сохраняем изменения
                Serial.println("Logic updated from /logic.json");
                runAutomation();
            }
        }
    }
    else
    {
        Serial.println("File /logic.json not found!");
    }
}
