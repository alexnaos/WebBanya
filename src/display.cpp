#include "config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
    {
        Serial.println(F("OLED init error!"));
    }
    display.clearDisplay();
    display.display();
}

void updateOLED()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // --- ВРЕМЯ ---
    display.setCursor(0, 0);
    Datime dt = sett.rtc;
    if (dt.hour < 10) display.print('0');
    display.print(dt.hour);
    display.print(':');
    if (dt.minute < 10) display.print('0');
    display.print(dt.minute);
    display.print(':');
    if (dt.second < 10) display.print('0');
    display.print(dt.second);

    if (millis() % 1000 < 500) display.print(" **");

    // --- СТАТУС WIFI ---
    display.setCursor(64, 0);
    display.print(WiFi.status() == WL_CONNECTED ? "ONLINE" : "OFFLINE");

    // --- СТАТУС И ВЫБОР ---
    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print("S:");
    display.print(db[kk::toggle] ? "ON" : "OFF");
    display.setCursor(70, 12);
    display.print("V:");
    display.print(db[kk::selectw].toInt() + 1);

    // --- СЛАЙДЕР ---
    display.setCursor(0, 30);
    display.print("R:");
    display.print(db[kk::slider].toFloat(), 1);

    // --- ТЕМПЕРАТУРА ---
    display.setCursor(0, 49);
    float t1 = db[kk::tmp1].toFloat();
    float t2 = db[kk::tmp2].toFloat();

    auto printTemp = [](float t)
    {
        if (t <= -50 || t >= 125 || t == 0)
            display.print("--");
        else
            display.print(t, 1);
    };

    printTemp(t1);
    display.print(" ");
    printTemp(t2);

    display.display();
}