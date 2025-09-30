#pragma once

#include <Arduino.h>
#include <IPAddress.h>

// Forward declarations to avoid including WiFiManager in headers
class WiFiManager;

class WiFiManagerWrapper {
public:
    WiFiManagerWrapper();
    ~WiFiManagerWrapper();

    void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    void setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn, IPAddress dns);
    void setConfigPortalTimeout(unsigned long seconds);
    bool autoConnect(const char* apName, const char* apPassword);
    void resetSettings();

private:
    WiFiManager* wifiManager;
};