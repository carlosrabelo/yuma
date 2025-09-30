#include "mdns_service.h"

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include "app_context.h"

void SetupMDNS() {
    // Initialize mDNS responder
    if (!MDNS.begin("yuma")) {
        Serial.println("Error setting up MDNS responder!");
        return;
    }

    Serial.println("mDNS responder started");
    Serial.println("Device available at: yuma.local");

    // Add service descriptions
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "device", "YUMA Stratum Proxy");
    MDNS.addServiceTxt("http", "tcp", "version", "1.0");
    MDNS.addServiceTxt("http", "tcp", "board", GetBoardName());

    // Add Stratum service
    MDNS.addService("stratum", "tcp", 3333);
    MDNS.addServiceTxt("stratum", "tcp", "device", "YUMA Stratum Proxy");
    MDNS.addServiceTxt("stratum", "tcp", "protocol", "stratum+tcp");
    MDNS.addServiceTxt("stratum", "tcp", "version", "1.0");

    Serial.println("mDNS services registered:");
    Serial.println("  - HTTP: http://yuma.local/");
    Serial.println("  - Stratum: stratum+tcp://yuma.local:3333");
}

void UpdateMDNS() {
#if defined(ESP8266)
    // ESP8266 requires periodic update
    MDNS.update();
#endif
    // ESP32 handles mDNS updates automatically
}