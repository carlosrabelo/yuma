#include "app_context.h"

#include <WiFiClient.h>

#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>

Config config = CreateDefaultConfig();
Metrics metrics{};
WiFiClient pool_client;
AsyncWebServer* server = nullptr;
AsyncServer stratum_server(4444);
std::vector<AsyncClient*> connected_miners;

const char* GetBoardName() {
#if defined(ESP32)
    return "ESP32";
#elif defined(ESP8266)
    return "ESP8266";
#else
    return "Unknown";
#endif
}
