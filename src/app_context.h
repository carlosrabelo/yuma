#pragma once

#include <vector>
#include <WiFiClient.h>

#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>

#include "config_manager.h"
#include "metrics.h"

extern Config config;
extern Metrics metrics;
extern WiFiClient pool_client;
extern AsyncWebServer* server;
extern AsyncServer stratum_server;
extern std::vector<AsyncClient*> connected_miners;

const char* GetBoardName();
