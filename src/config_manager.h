#pragma once

#include <Arduino.h>

struct Config {
    char ssid[32];
    char password[64];
    char pool_host[64];
    int pool_port;
    char pool_user[64];
    char pool_pass[32];
    int difficulty;
    bool vardiff_enabled;
    int vardiff_target;
    int vardiff_min;
    int vardiff_max;
    bool use_static_ip;
    char static_ip[16];
    char static_gateway[16];
    char static_subnet[16];
    char static_dns[16];
};

Config CreateDefaultConfig();
void CopyStringField(char* dest, size_t dest_size, const String& value);

bool LoadConfig(Config& cfg);
bool SaveConfig(const Config& cfg);
