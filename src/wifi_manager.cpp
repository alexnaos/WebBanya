#include "config.h"

ESP8266WiFiMulti wifiMulti;

void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.hostname(MDNS_NAME); // Устанавливаем hostname для DHCP

    wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
    wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

    Serial.println("WiFi: Connecting...");

    unsigned long start = millis();
    while (wifiMulti.run() != WL_CONNECTED)
    {
        delay(100);
        if (millis() - start >= WIFI_TIMEOUT_MS)
        {
            Serial.println("\nWiFi: Timeout! Working offline.");
            return;
        }
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected to: " + WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // mDNS
    if (MDNS.begin(MDNS_NAME)) {
        Serial.printf("[mDNS] OK: %s.local\n", MDNS_NAME);
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("[mDNS] FAIL");
    }
}
