# WebBanya3 — Система управления баней на ESP8266

Прошивка для **Wemos D1 Mini (ESP8266)** для управления нагревом и мониторинга бани/отопления с веб-интерфейсом, MQTT (с Home Assistant) и OLED-дисплеем.

## 📋 Основные возможности

- **Датчики температуры:** 2× DS18B20 на шине OneWire (внутри/снаружи)
- **OLED дисплей:** 128×64 SSD1306 (I2C) — часы, статус, температуры
- **Часы реального времени:** DS3231 (I2C) с синхронизацией через веб-интерфейс
- **Управление нагрузкой:** ШИМ-регулировка мощности (D7, 10 кГц, 0–1023)
- **Веб-интерфейс:** GyverDB + SettingsGyver — настройка через браузер
- **MQTT + Home Assistant:** ручная интеграция через `configuration.yaml`
- **Multi-WiFi:** Автоматическое переключение между двумя точками доступа
- **OTA:** Обновление прошивки по Wi-Fi
- **Автоматизация:** вся логика сценариев вынесена в Home Assistant (ESP только публикует состояние и исполняет команды)

## 🔧 Аппаратная часть

| Компонент       | Пин        | Примечание              |
|-----------------|------------|-------------------------|
| DS18B20 (data)  | D5 (GPIO14)| 2 датчика на одной шине |
| OLED SSD1306    | I2C (D1, D2)| Адрес 0x3C              |
| DS3231 RTC      | I2C (D1, D2)| Часы реального времени  |
| MOSFET/SSR (PWM)| D7 (GPIO13)| 10 кГц, 10 бит (0–1023) |

## 📦 Зависимости (PlatformIO)

Все указаны в `platformio.ini`:
- `GyverDB` — база данных с автосохранением
- `Settings` — веб-интерфейс
- `OneWire` + `DallasTemperature` — датчики DS18B20
- `Adafruit GFX` + `Adafruit SSD1306` — OLED дисплей
- `GyverDS3231` — часы реального времени
- `ArduinoJson` — работа с JSON (MQTT)
- `PubSubClient` — MQTT

## 📁 Структура проекта

```
WebBanya3/
├── platformio.ini            # Конфигурация PlatformIO
├── src/
│   ├── main.cpp              # Setup + главный loop
│   ├── config.h              # Конфигурация, пины, extern-объявления
│   ├── wifi_manager.cpp      # Multi-WiFi с таймаутом
│   ├── mqtt_manager.cpp      # MQTT публикация/подписка
│   ├── sensors.cpp           # Датчики DS18B20
│   ├── display.cpp           # OLED дисплей
│   ├── control.cpp           # ШИМ управление нагрузкой
│   ├── rtc_manager.cpp       # Часы DS3231 + синхронизация
│   ├── ota_manager.cpp       # OTA-обновления
│   ├── ui_portal.cpp         # Веб-интерфейс (SettingsGyver)
│   └── ui_portal.h           # Заголовок веб-интерфейса
├── configuration.yaml        # HA config — скопировать в HA
├── ha_configuration.yaml     # Резервная копия блока MQTT для HA
├── ha_dashboard_card.yaml    # Карточка для дашборда HA
├── archive/
│   └── work.cpp              # Предыдущая версия (не используется)
├── data/
│   └── logic.json            # Больше не читается ESP
├── README.md
└── OPTIMIZATION.md
```

## 🚀 Быстрый старт

1. Установите [PlatformIO](https://platformio.org/) в VS Code
2. Склонируйте репозиторий:
   ```bash
   git clone https://github.com/alexnaos/WebBanya.git
   cd WebBanya3
   ```
3. Отредактируйте `src/config.h`:
   - Настройте SSID и пароли Wi-Fi
   - Укажите адрес MQTT-сервера
   - При необходимости — адреса датчиков DS18B20
4. Соберите и загрузите прошивку:
   ```bash
   pio run --target upload           # Первая прошивка по USB
   pio run --target upload --upload-port "WebBanya3.local"  # OTA
   ```

## 🌐 Веб-интерфейс

После подключения к Wi-Fi откройте IP-адрес ESP в браузере.

Страница содержит:
- Отображение температур (Дом / Улица)
- Управление реле (вкл/выкл)
- Слайдер мощности (0–1023)
- Выбор режима
- Настройка даты/времени (синхронизация с RTC)
- Системные кнопки (reload, clear db)

## 📡 MQTT

**Публикация** (JSON, каждые 2 сек):
- Топик: `esp/state`
- Поля: `temp1`, `temp2`, `slider`, `toggle`, `select`, `select_mode`, `uptime`

**Управление** (подписка):
- `esp/set/slider` — мощность (число)
- `esp/set/toggle` — реле (`ON`/`OFF`)
- `esp/set/select` — режим (число или имя опции)

## 🏠 Home Assistant

### Подключение в 2 шага

1. **Скопируй `configuration.yaml`** в папку `/config/` на сервере HA
2. **Перезагрузи HA:** Настройки → Система → Управление → Перезагрузить

После перезагрузки появится устройство **WebBanya3** с 5 сущностями:

| Сущность | Тип | Описание |
|---|---|---|
| `sensor.webbanya3_temp1` | Температура | Датчик дом |
| `sensor.webbanya3_temp2` | Температура | Датчик улица |
| `switch.webbanya3_relay` | Выключатель | Реле |
| `number.webbanya3_power` | Число (слайдер) | Мощность 0–1023 |
| `select.webbanya3_mode` | Выбор | Режим (var1/var2/hello) |

### Карточка на дашборд

Используй файл `ha_dashboard_card.yaml`:
1. HA → Обзор → ⋮ → Редактировать панель
2. + Добавить карточку → вкладка "Редактор кода"
3. Вставь содержимое `ha_dashboard_card.yaml` → Сохранить

### Автоматизация

Вся логика сценариев (расписание, температура, дни недели) вынесена в Home Assistant.
Пример автоматизации:

```yaml
alias: "Баня: управление по температуре"
trigger:
  - platform: numeric_state
    entity_id: sensor.webbanya3_temp1
    below: 22
action:
  - service: switch.turn_on
    target:
      entity_id: switch.webbanya3_relay
  - service: number.set_value
    target:
      entity_id: number.webbanya3_power
    data:
      value: 512
```

## 🔧 Конфигурация

Все настройки в одном файле `src/config.h`:

| Макрос           | Значение    | Описание                     |
|------------------|-------------|-------------------------------|
| `WIFI_SSID_1`    | Ban2        | Основная точка доступа Wi-Fi  |
| `WIFI_TIMEOUT_MS`| 15000       | Таймаут подключения (мс)      |
| `MQTT_SERVER`    | 192.168.1.23| Адрес MQTT-брокера            |
| `TIMEZONE`       | 3           | Часовой пояс (МСК)            |
| `ONE_WIRE_BUS`   | D5          | Пин датчиков DS18B20          |
| `PWM_MAX`        | 1023        | Максимум ШИМ (10 бит)         |

## 📄 Лицензия

MIT