#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "app_context.h"
#include "config_manager.h"
#include "mdns_service.h"
#include "pool_client.h"
#include "status_display.h"
#include "storage.h"
#include "stratum_server.h"
#include "web_interface.h"
#include "wifi_setup.h"

static bool services_initialized = false;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== YUMA Stratum Proxy ===");
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

    // Only setup services after WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
        SetupMDNS();
        SetupWebServer();
        SetupStratumServer();
        services_initialized = true;
        Serial.println("System initialized!");
    } else {
        Serial.println("WiFi not connected, services will start after connection");
    }
}

void loop() {
    static unsigned long last_wifi_check = 0;

    if (WiFi.status() != WL_CONNECTED && millis() - last_wifi_check > 10000) {
        last_wifi_check = millis();
        Serial.printf("WiFi disconnected (status: %d), attempting reconnection...\n", WiFi.status());

        // Try to reconnect with saved credentials
        WiFi.begin();

        // Wait for connection with timeout
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi reconnected!");
            DebugWifiStatus();
        } else {
            Serial.println("\nFailed to reconnect automatically");
            Serial.println("To reconfigure WiFi, restart device or call ResetWifiSettings()");
        }
        return;
    }

    // Initialize services once WiFi is connected
    if (!services_initialized && WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected! Initializing services...");
        SetupMDNS();
        SetupWebServer();
        SetupStratumServer();
        services_initialized = true;
        Serial.println("Services initialized!");
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
    UpdateMDNS();

    delay(100);
}
