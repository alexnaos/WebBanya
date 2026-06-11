# WebBanya3 — Система управления баней на ESP8266

Прошивка для **Wemos D1 Mini (ESP8266)** для управления нагревом и мониторинга бани/отопления с веб-интерфейсом, MQTT (с Home Assistant Discovery) и OLED-дисплеем.

## 📋 Основные возможности

- **Датчики температуры:** 2× DS18B20 на шине OneWire (внутри/снаружи)
- **OLED дисплей:** 128×64 SSD1306 (I2C) — часы, статус, температуры
- **Часы реального времени:** DS3231 (I2C) с синхронизацией через веб-интерфейс
- **Управление нагрузкой:** ШИМ-регулировка мощности (D7, 10 кГц, 0–1023)
- **Веб-интерфейс:** GyverDB + SettingsGyver — настройка через браузер
- **MQTT + Home Assistant Discovery:** авто-регистрация сенсоров в HA
- **Multi-WiFi:** Автоматическое переключение между двумя точками доступа
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
- `ArduinoJson` — работа с JSON (MQTT, HA Discovery)
- `PubSubClient` — MQTT
- `GyverBME280` — опционально, зарезервировано

## 📁 Структура проекта (после рефакторинга)

```
WebBanya3/
├── platformio.ini          # Конфигурация PlatformIO
├── src/
│   ├── main.cpp            # Setup + главный loop
│   ├── config.h            # Конфигурация, пины, extern-объявления
│   ├── wifi_manager.cpp    # Multi-WiFi с таймаутом
│   ├── mqtt_manager.cpp    # MQTT + Home Assistant Discovery
│   ├── sensors.cpp         # Датчики DS18B20
│   ├── display.cpp         # OLED дисплей
│   ├── control.cpp         # ШИМ управление нагрузкой
│   ├── rtc_manager.cpp     # Часы DS3231 + синхронизация
│   ├── ui_portal.cpp       # Веб-интерфейс (SettingsGyver)
│   └── ui_portal.h         # Заголовок веб-интерфейса
├── data/
│   └── logic.json          # ⚠️ Больше не читается ESP — оставлен для совместимости
├── archive/
│   └── work.cpp            # Предыдущая версия, перемещена из lib/
├── test/                   # Тесты (пусто)
├── include/                # Дополнительные заголовки
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
4. Если есть файловая система `data/` — загрузите:
   ```bash
   pio run --target uploadfs
   ```
5. Соберите и загрузите прошивку:
   ```bash
   pio run --target upload
   ```

## 🌐 Веб-интерфейс

После подключения к Wi-Fi откройте IP-адрес ESP в браузере.

Страница содержит:
- Отображение температур (Дом / Улица)
- Управление реле (вкл/выкл)
- Слайдер мощности (0–1023)
- Выбор режима
- Настройка даты/времени (синхронизация с RTC)
- Дата/время с RTC
- Системные кнопки (reload, clear db)

## 📡 MQTT + Home Assistant

### Home Assistant Discovery (авто-регистрация)

При подключении к MQTT ESP публикует конфигурацию для HA Discovery:

| Сущность          | Тип      | Топик                         |
|-------------------|----------|-------------------------------|
| Температура дом   | sensor   | `homeassistant/sensor/.../config` |
| Температура улица | sensor   | `homeassistant/sensor/.../config` |
| Реле              | switch   | `homeassistant/switch/.../config` |
| Мощность          | number   | `homeassistant/number/.../config` |
| Режим             | select   | `homeassistant/select/.../config` |

Все сущности объединены в одно устройство `WebBanya3` с ID `webbanya3_esp`.

### Публикация состояния (JSON, каждые 2 сек)
- Топик: `esp/state`
- Поля: `temp1`, `temp2`, `slider`, `toggle`, `select`, `uptime`

### Управление (подписка)
- `esp/set/slider` — мощность (число)
- `esp/set/toggle` — реле (`1`/`0`, `true`/`false`, `ON`/`OFF`)
- `esp/set/select` — режим (число 0, 1, 2)

### 🏠 Автоматизация в Home Assistant

Вся логика сценариев (расписание, температура, дни недели) **вынесена в Home Assistant**. Используйте **Node-RED** или **Автоматизации HA**:

```yaml
# Пример автоматизации HA (YAML)
alias: "Баня: управление по температуре"
trigger:
  - platform: numeric_state
    entity_id: sensor.webbanya3_esp_temp1
    below: 22
action:
  - service: switch.turn_on
    target:
      entity_id: switch.webbanya3_esp_relay
  - service: number.set_value
    target:
      entity_id: number.webbanya3_esp_power
    data:
      value: 512
```

## ⚙️ Сценарии автоматизации (устаревшее)

Файл `data/logic.json` больше **не обрабатывается** ESP. Вся автоматизация перенесена в Home Assistant / Node-RED через MQTT.

## 🔧 Конфигурация

Все настройки в одном файле `src/config.h`:

| Макрос           | Значение по умолч. | Описание                     |
|------------------|-------------------|-------------------------------|
| `WIFI_SSID_1`    | Ban2              | Основная точка доступа Wi-Fi  |
| `WIFI_PASS_1`    | ...               | Пароль                        |
| `WIFI_SSID_2`    | Sloboda100        | Резервная точка доступа       |
| `WIFI_TIMEOUT_MS`| 15000             | Таймаут подключения (мс)      |
| `MQTT_SERVER`    | 192.168.1.23      | Адрес MQTT-брокера            |
| `MQTT_PORT`      | 1883              | Порт MQTT                     |
| `TIMEZONE`       | 3                 | Часовой пояс (МСК)            |
| `ONE_WIRE_BUS`   | D5                | Пин датчиков DS18B20          |
| `PWM_MAX`        | 1023              | Максимум ШИМ (10 бит)         |

## 📄 Лицензия

MIT