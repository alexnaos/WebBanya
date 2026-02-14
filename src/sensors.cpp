#include "config.h"

// Инициализация объектов (объявлены как extern в config.h)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

static uint32_t tmr_temp;

void initSensors() {
    sensors.begin();
    sensors.setWaitForConversion(false); // Чтобы не блокировать цикл
    sensors.requestTemperatures();
}

void handleSensors() {
    // Опрос по таймеру
    if (millis() - tmr_temp >= TEMP_UPDATE_INTERVAL) {
        tmr_temp = millis();

        // 1. Читаем температуру из датчиков в глобальные переменные (для OLED)
        temp1 = sensors.getTempCByIndex(0);
        temp2 = sensors.getTempCByIndex(1);

        // 2. ЗАПИСЬ В БАЗУ ДАННЫХ (GyverDB)
        // Теперь эти значения будут автоматически подхватываться веб-интерфейсом
        db[kk::tmp1] = temp1;
        db[kk::tmp2] = temp2;

        // 3. Отправляем запрос на следующее измерение
        sensors.requestTemperatures();
        
        // Для отладки
        // Serial.printf("T1: %.2f, T2: %.2f\n", temp1, temp2);
    }
}
