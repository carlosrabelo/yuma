#include "wifi_setup.h"

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "app_context.h"
#include "wifi_manager_wrapper.h"

namespace {
WiFiManagerWrapper wifiManager;
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

    wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1),
                                    IPAddress(255, 255, 255, 0));
    wifiManager.setConfigPortalTimeout(300);

    WiFi.persistent(false);
#if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
    WiFi.setSleep(false);
#endif

    bool static_configured = ConfigureStaticIp(static_ip, static_gateway, static_subnet, static_dns);
    if (static_configured) {
        Serial.printf("Static IP requested: %s\n", static_ip.toString().c_str());
    }

    if (!wifiManager.autoConnect("YUNA-StratumProxy", "12345678")) {
        Serial.println("Failed to connect WiFi and portal timeout");
        ESP.restart();
    }

    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    if (static_configured) {
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
    }
}

void ResetWifiSettings() {
    wifiManager.resetSettings();
}
