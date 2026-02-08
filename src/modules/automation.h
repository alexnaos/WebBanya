#ifndef AUTOMATION_H
#define AUTOMATION_H

#include "../config/config.h" // Подключаем общие объявления и библиотеки
extern float temp1; // <-- Добавьте эту строку
extern float temp2; // <-- Добавьте эту строку

// Объявления функций, реализованных в automation.cpp
void runAutomation();
void loadLogicFromFile();

#endif // AUTOMATION_H
