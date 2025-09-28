#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "app_context.h"
#include "config_manager.h"
#include "pool_client.h"
#include "status_display.h"
#include "storage.h"
#include "stratum_server.h"
#include "web_interface.h"
#include "wifi_setup.h"

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== YUNA Stratum Proxy ===");
    Serial.printf("Target board: %s\n", GetBoardName());

#ifdef USE_OLED_STATUS
    SetupStatusDisplay();
#endif

    metrics.uptime_start = millis();

    if (!SetupStorage()) {
        Serial.println("Storage initialization failed; continuing with defaults");
    }

    LoadConfig(config);
    SetupWifi();
    SetupWebServer();
    SetupStratumServer();

    Serial.println("System initialized!");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, trying to reconnect...");
        WiFi.disconnect(false);
        WiFi.begin();
        delay(5000);
        return;
    }

    if (ShouldConnectToPool() && !pool_client.connected()) {
        ConnectToPool();
    }

    if (pool_client.connected() && connected_miners.empty()) {
        DisconnectFromPool();
    }

    if (pool_client.connected()) {
        HandlePoolData();
    }

#ifdef USE_OLED_STATUS
    UpdateStatusDisplay();
#endif

    HandleMinerConnections();

    delay(100);
}
