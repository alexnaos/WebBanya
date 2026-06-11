#include "config.h"

void handleControl()
{
    static uint32_t tmrControl;
    static int lastVal = -1;

    if (millis() - tmrControl >= 100)
    {
        tmrControl = millis();

        int currentVal = db[kk::toggle].toBool() ? db[kk::slider].toInt() : 0;
        currentVal = constrain(currentVal, 0, PWM_MAX);

        if (currentVal != lastVal)
        {
            analogWrite(LED_PIN, currentVal);
            lastVal = currentVal;
        }
    }
}