#include "wifi_manager_wrapper.h"

// Include WiFiManager only in the implementation file
#include <WiFiManager.h>

WiFiManagerWrapper::WiFiManagerWrapper() {
    wifiManager = new WiFiManager();
}

WiFiManagerWrapper::~WiFiManagerWrapper() {
    delete wifiManager;
}

void WiFiManagerWrapper::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn) {
    wifiManager->setAPStaticIPConfig(ip, gw, sn);
}

void WiFiManagerWrapper::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn, IPAddress dns) {
    wifiManager->setSTAStaticIPConfig(ip, gw, sn, dns);
}

void WiFiManagerWrapper::setConfigPortalTimeout(unsigned long seconds) {
    wifiManager->setConfigPortalTimeout(seconds);
}

bool WiFiManagerWrapper::autoConnect(const char* apName, const char* apPassword) {
    return wifiManager->autoConnect(apName, apPassword);
}

void WiFiManagerWrapper::resetSettings() {
    wifiManager->resetSettings();
}