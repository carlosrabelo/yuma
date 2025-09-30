#include "wifi_setup.h"

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <WiFiManager.h>
#include "app_context.h"

namespace {
WiFiManager wifiManager;

constexpr char kPortalSsid[] = "YUMA-PROXY";
constexpr char kPortalPassword[] = "12345678";
constexpr uint8_t kPortalMaxRetries = 3;

bool WaitForConnection(unsigned long timeout_ms) {
    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
        delay(100);
    }
    return WiFi.status() == WL_CONNECTED;
}

#if defined(ESP32)
void on_wifi_disconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.printf("WiFi disconnected. Reason: %d\n", info.wifi_sta_disconnected.reason);
    // Full list of reasons:
    // https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_wifi_types.h
}
#endif
}

bool ConfigureStaticIp(IPAddress& ip_out, IPAddress& gateway_out,
                       IPAddress& subnet_out, IPAddress& dns_out) {
    Config& cfg = config;
    if (!cfg.use_static_ip) {
        return false;
    }

    auto parse_ip = [](const char* value, IPAddress& out) -> bool {
        if (!value || value[0] == '\0') {
            return false;
        }
        return out.fromString(value);
    };

    if (!parse_ip(cfg.static_ip, ip_out) || !parse_ip(cfg.static_gateway, gateway_out) ||
        !parse_ip(cfg.static_subnet, subnet_out)) {
        Serial.println("Invalid static IP configuration detected, falling back to DHCP");
        return false;
    }

    if (!parse_ip(cfg.static_dns, dns_out)) {
        dns_out = gateway_out;
    }

    wifiManager.setSTAStaticIPConfig(ip_out, gateway_out, subnet_out, dns_out);
#if defined(ESP8266) || defined(ESP32)
    WiFi.config(ip_out, gateway_out, subnet_out, dns_out);
#endif
    return true;
}

void SetupWifi() {
    IPAddress static_ip;
    IPAddress static_gateway;
    IPAddress static_subnet;
    IPAddress static_dns;

#if defined(ESP32)
    // Register the disconnect handler
    WiFi.onEvent(on_wifi_disconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Configure access point settings
    wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                                    IPAddress(255, 255, 255, 0));
    wifiManager.setConfigPortalTimeout(300);  // 5 minutes
    wifiManager.setConnectTimeout(30);
    wifiManager.setConnectRetries(3);
    wifiManager.setBreakAfterConfig(true);
    wifiManager.setWiFiAutoReconnect(true);

    // Enable debug info
    wifiManager.setDebugOutput(true);

    // Save parameters to flash automatically
    wifiManager.setSaveParamsCallback([]() {
        Serial.println("WiFi parameters saved!");
    });

    // IMPORTANT: Allow WiFi credentials to be saved
    WiFi.persistent(true);
#if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
    WiFi.setSleep(false);
#endif

    // Configure static IP if requested
    bool static_configured = ConfigureStaticIp(static_ip, static_gateway, static_subnet, static_dns);
    if (static_configured) {
        Serial.printf("Static IP requested: %s\n", static_ip.toString().c_str());
    }

    Serial.println("Starting WiFi connection...");

    bool connected = WiFi.status() == WL_CONNECTED;
    if (!connected) {
        connected = wifiManager.autoConnect(kPortalSsid, kPortalPassword);
    }

    if (WiFi.status() != WL_CONNECTED) {
        connected = WaitForConnection(10000);
    }

    if (!connected || WiFi.status() != WL_CONNECTED) {
        Serial.println("Connection failed after autoConnect, reopening config portal...");
        uint8_t attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < kPortalMaxRetries) {
            attempt++;
            Serial.printf("Config portal attempt %u/%u\n",
                          static_cast<unsigned int>(attempt),
                          static_cast<unsigned int>(kPortalMaxRetries));
            bool portal_connected = wifiManager.startConfigPortal(kPortalSsid, kPortalPassword);
            if (portal_connected && WaitForConnection(10000)) {
                break;
            }
            Serial.println("Config portal closed without a successful connection");
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Unable to establish WiFi link after multiple attempts. Restarting...");
            delay(1000);
            ESP.restart();
        }
    }

    Serial.println("WiFi connected!");
    DebugWifiStatus();
}

void ResetWifiSettings() {
    Serial.println("Resetting WiFi settings...");
    wifiManager.resetSettings();

    // Also clear ESP32/ESP8266 saved credentials
    WiFi.disconnect(true);  // true = erase saved credentials
    delay(1000);
}

void DebugWifiStatus() {
    Serial.printf("WiFi Status: %d\n", WiFi.status());
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
}
