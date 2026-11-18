#ifndef ENCODER_MENU_H
#define ENCODER_MENU_H

#include "config.h"
#include <GyverEncoder.h>

// Пины энкодера
#define ENC_CLK D1
#define ENC_DT  D2
#define ENC_SW  D3

// Режимы дисплея
enum class DisplayMode : uint8_t {
    MAIN,       // Основной экран (время, статус, температуры)
    MENU_NAV,   // Навигация по пунктам меню
    MENU_EDIT,  // Редактирование выбранного пункта
};

// Пункты меню
enum class MenuItem : uint8_t {
    RELAY = 0,
    POWER,
    MODE,
    TEMP1,
    TEMP2,
    COUNT
};

// Глобальные переменные меню
extern Encoder enc;
extern DisplayMode displayMode;
extern MenuItem currentMenuItem;

// Функции
void initEncoder();
void handleEncoder();
void updateOLED_Menu();

#endif