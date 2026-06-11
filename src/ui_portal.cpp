#include "ui_portal.h"

void build(sets::Builder &b)
{
    if (b.build.isAction())
    {
        logger.print("Set: 0x");
        logger.println(b.build.id, HEX);
        b.DateTime("rtc");
    }
    b.Label("Текущее время RTC: " + sett.rtc.toString());

    if (b.beginGroup("Group 1"))
    {
        b.Input(kk::txt, "Text");
        b.Label(kk::tmp1, "Дом");
        b.Label(kk::tmp2, "Улица");
        b.endGroup();
    }
    b.Switch(kk::toggle, "Реле");
    b.Slider(kk::slider, "Мощность", 0, 1023, 1);
    b.Select(kk::selectw, "Выбор", "var1;var2;hello");

    if (b.beginRow())
    {
        if (b.Button("click"))
        {
            Serial.println("click: " + String(b.build.pressed()));
        }
        if (b.ButtonHold("hold"))
        {
            Serial.println("hold: " + String(b.build.pressed()));
        }
        b.endRow();
    }
    if (b.beginGroup("Group3"))
    {
        b.Date(kk::date, "Date");
        b.Time(kk::timew, "Time");
        b.DateTime(kk::datime, "Datime");
        b.endGroup();
    }
    if (b.beginGroup("Система управления"))
    {
        if (b.beginButtons())
        {
            if (b.Button(kk::btn1, "reload"))
            {
                Serial.println("reload");
                b.reload();
            }
            if (b.Button(kk::btn2, "clear db", sets::Colors::Blue))
            {
                Serial.println("clear db");
                db.update();
            }
            b.endButtons();
        }
        b.endGroup();
    }
}

void update(sets::Updater &upd)
{
    upd.update(kk::lbl1, random(100));
    upd.update(kk::lbl2, millis());
    upd.update(kk::tmp1, db[kk::tmp1].toFloat(), 2);
    upd.update(kk::tmp2, db[kk::tmp2].toFloat(), 2);

    // Синхронизация RTC с БД раз в минуту
    static uint32_t syncTmr = 0;
    if (millis() - syncTmr >= 60000)
    {
        syncTmr = millis();
        syncRTCFromDB();
    }
}