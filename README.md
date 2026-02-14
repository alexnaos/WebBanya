
WebBanya/
├── data/              # Файлы для LittleFS (data.db, иконки)
├── src/
│   ├── main.cpp       # Только setup(), loop() и системные вызовы
│   ├── config.h       # Пины, константы, extern объекты
│   ├── ui_portal.cpp  # Веб-интерфейс
│   ├── sensors.cpp    # Работа с температурой
│   └── automation.cpp # Логика управления
└── platformio.ini     # Настройки библиотек

