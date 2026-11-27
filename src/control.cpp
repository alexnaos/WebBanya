#include "config.h"

void handleControl()
{
    static uint32_t tmrControl;
    static int lastVal = -1;

    if (millis() - tmrControl >= 100)
    {
        tmrControl = millis();

        // Ползунок мощности задаёт ШИМ напрямую, его диапазон: MIN_FAN_PWM..PWM_MAX
        int rawVal = db[kk::slider].toInt();
        bool isOn = db[kk::toggle].toBool(); // Тумблер Вкл/Выкл

        int currentVal = 0;

        if (isOn)
        {
            // Ниже MIN_FAN_PWM вентилятор не стартует, поэтому не опускаемся ниже
            currentVal = constrain(rawVal, MIN_FAN_PWM, PWM_MAX);

            // Если в БД осталось старое значение вне диапазона (0, -3.5 и т.п.) —
            // приводим его к тому, что реально выдаём, чтобы веб и MQTT не врали
            if (rawVal != currentVal) db[kk::slider] = currentVal;
        }

        if (currentVal != lastVal)
        {
            // У вас в config.h LED_PIN = D7. Если вентилятор подключен к D7 — всё верно.
            analogWrite(LED_PIN, currentVal);
            lastVal = currentVal;
        }
    }
}
