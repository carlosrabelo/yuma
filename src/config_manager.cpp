#include "config_manager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

#include "config_defaults.h"
#include "platform_fs.h"
#include "storage.h"

namespace {
void CopyLiteral(char* dest, size_t size, const char* src) {
    if (!dest || size == 0) {
        return;
    }

    if (!src) {
        dest[0] = '\0';
        return;
    }

    std::strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}
}

Config CreateDefaultConfig() {
    Config cfg{};

    cfg.ssid[0] = '\0';
    cfg.password[0] = '\0';

    CopyLiteral(cfg.pool_host, sizeof(cfg.pool_host), ConfigDefaults::kPoolHost);
    cfg.pool_port = ConfigDefaults::kPoolPort;
    CopyLiteral(cfg.pool_user, sizeof(cfg.pool_user), ConfigDefaults::kPoolUser);
    CopyLiteral(cfg.pool_pass, sizeof(cfg.pool_pass), ConfigDefaults::kPoolPass);
    cfg.difficulty = ConfigDefaults::kDifficulty;
    cfg.vardiff_enabled = ConfigDefaults::kVardiffEnabled;
    cfg.vardiff_target = ConfigDefaults::kVardiffTarget;
    cfg.vardiff_min = ConfigDefaults::kVardiffMin;
    cfg.vardiff_max = ConfigDefaults::kVardiffMax;
    cfg.use_static_ip = ConfigDefaults::kUseStaticIp;
    CopyLiteral(cfg.static_ip, sizeof(cfg.static_ip), ConfigDefaults::kStaticIp);
    CopyLiteral(cfg.static_gateway, sizeof(cfg.static_gateway), ConfigDefaults::kStaticGateway);
    CopyLiteral(cfg.static_subnet, sizeof(cfg.static_subnet), ConfigDefaults::kStaticSubnet);
    CopyLiteral(cfg.static_dns, sizeof(cfg.static_dns), ConfigDefaults::kStaticDns);

    return cfg;
}

void CopyStringField(char* dest, size_t dest_size, const String& value) {
    if (!dest || dest_size == 0) {
        return;
    }

    String trimmed = value;
    trimmed.trim();
    trimmed.toCharArray(dest, dest_size);
    if (trimmed.length() == 0) {
        dest[0] = '\0';
    }
}

bool LoadConfig(Config& cfg) {
    if (!EnsureStorageMounted()) {
        Serial.println("Storage not mounted, using default configuration");
        cfg = CreateDefaultConfig();
        return false;
    }

    File file = STORAGE_FS.open("/config.json", "r");
    if (!file) {
        Serial.println("Configuration file not found, using defaults");
        cfg = CreateDefaultConfig();
        return false;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err != DeserializationError::Ok) {
        Serial.printf("Failed to parse configuration (%s), using defaults\n", err.c_str());
        cfg = CreateDefaultConfig();
        return false;
    }

    cfg = CreateDefaultConfig();

    CopyLiteral(cfg.pool_host, sizeof(cfg.pool_host), doc["pool_host"] | ConfigDefaults::kPoolHost);
    cfg.pool_port = doc["pool_port"] | ConfigDefaults::kPoolPort;
    CopyLiteral(cfg.pool_user, sizeof(cfg.pool_user), doc["pool_user"] | ConfigDefaults::kPoolUser);
    CopyLiteral(cfg.pool_pass, sizeof(cfg.pool_pass), doc["pool_pass"] | ConfigDefaults::kPoolPass);
    cfg.difficulty = doc["difficulty"] | ConfigDefaults::kDifficulty;
    cfg.vardiff_enabled = doc["vardiff_enabled"] | ConfigDefaults::kVardiffEnabled;
    cfg.vardiff_target = doc["vardiff_target"] | ConfigDefaults::kVardiffTarget;
    cfg.vardiff_min = doc["vardiff_min"] | ConfigDefaults::kVardiffMin;
    cfg.vardiff_max = doc["vardiff_max"] | ConfigDefaults::kVardiffMax;
    cfg.use_static_ip = doc["use_static_ip"] | ConfigDefaults::kUseStaticIp;

    CopyLiteral(cfg.static_ip, sizeof(cfg.static_ip), doc["static_ip"] | ConfigDefaults::kStaticIp);
    CopyLiteral(cfg.static_gateway, sizeof(cfg.static_gateway), doc["static_gateway"] | ConfigDefaults::kStaticGateway);
    CopyLiteral(cfg.static_subnet, sizeof(cfg.static_subnet), doc["static_subnet"] | ConfigDefaults::kStaticSubnet);
    CopyLiteral(cfg.static_dns, sizeof(cfg.static_dns), doc["static_dns"] | ConfigDefaults::kStaticDns);

    Serial.println("Configuration loaded");
    Serial.printf("Static IP: %s, gateway: %s, subnet: %s, dns: %s, enabled: %s\n",
                  cfg.static_ip,
                  cfg.static_gateway,
                  cfg.static_subnet,
                  cfg.static_dns,
                  cfg.use_static_ip ? "yes" : "no");

    return true;
}

bool SaveConfig(const Config& cfg) {
    if (!EnsureStorageMounted()) {
        Serial.println("Storage not mounted, cannot save configuration");
        return false;
    }

    DynamicJsonDocument doc(1024);

    doc["pool_host"] = cfg.pool_host;
    doc["pool_port"] = cfg.pool_port;
    doc["pool_user"] = cfg.pool_user;
    doc["pool_pass"] = cfg.pool_pass;
    doc["difficulty"] = cfg.difficulty;
    doc["vardiff_enabled"] = cfg.vardiff_enabled;
    doc["vardiff_target"] = cfg.vardiff_target;
    doc["vardiff_min"] = cfg.vardiff_min;
    doc["vardiff_max"] = cfg.vardiff_max;
    doc["use_static_ip"] = cfg.use_static_ip;
    doc["static_ip"] = cfg.static_ip;
    doc["static_gateway"] = cfg.static_gateway;
    doc["static_subnet"] = cfg.static_subnet;
    doc["static_dns"] = cfg.static_dns;

    if (STORAGE_FS.exists("/config.json")) {
        STORAGE_FS.remove("/config.json");
    }

    File file = STORAGE_FS.open("/config.json", "w");
#if defined(ESP32)
    if (!file) {
        file = STORAGE_FS.open("/config.json", FILE_WRITE);
    }
#endif
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    size_t written = serializeJson(doc, file);
    file.close();

    if (written == 0) {
        Serial.println("Failed to write configuration to file");
        return false;
    }

    Serial.println("Configuration saved");
    Serial.printf("Static IP saved as %s (enabled=%s)\n", cfg.static_ip, cfg.use_static_ip ? "yes" : "no");
    return true;
}
