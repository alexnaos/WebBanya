#include "ui_portal.h"

// Описываем интерфейс

void build(sets::Builder &b)
{
    if (b.build.isAction())
    {
        logger.print("Set: 0x");
        logger.println(b.build.id, HEX);
        b.DateTime("rtc"); // Дата и время в одном флаконе
    }
    b.Label("Текущее время RTC: " + sett.rtc.toString());

    if (b.beginGroup("Group 1")) // Группа 1
    {
        b.Input(kk::txt, "Text");
        // b.Pass(kk::pass, "Password");
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
    if (b.beginGroup("Group3")) // Группа с датами и временем
    {
        b.Date(kk::date, "Date");
        b.Time(kk::timew, "Time");
        b.DateTime(kk::datime, "Datime");
        b.endGroup(); // Закрываем Group3
    }
    // --- Блок системных кнопок ---
    if (b.beginGroup("Система управления")) // Переименовано для уникальности ID
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
                // db.clear();
                db.update();
            }
            b.endButtons();
        }
        b.endGroup(); // Закрываем Систему управления
    }
}

void update(sets::Updater &upd)
{
    upd.update(kk::lbl1, random(100));
    upd.update(kk::lbl2, millis());
    // upd.update(kk::tmp1, temp1);
    // upd.update(kk::tmp2, temp2);
    upd.update(kk::tmp1, db[kk::tmp1].toFloat(), 2);
    upd.update(kk::tmp2, db[kk::tmp2].toFloat(), 2);

    // Берем значение из БД (куда его сохранил виджет DateTime)
    uint32_t dbUnix = db[kk::date].toInt32();

    // Если время в базе отличается от системного более чем на 5 секунд
    if (abs((long)dbUnix - (long)time(NULL)) > 5)
    {
        rtc.setUnix(dbUnix);
        timeval tv = {(time_t)dbUnix, 0};
        settimeofday(&tv, NULL);
        Serial.println("RTC: Синхронизировано по разнице значений!");
    }
}
