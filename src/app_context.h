#pragma once

#include <vector>

#include <WiFiClient.h>

#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif

#ifdef ESP8266
#define HTTP_ANY HTTP_ANY_AWS
#endif

#if defined(ESP8266) || defined(ESP32)
#define HTTP_GET HTTP_GET_AWS
#define HTTP_HEAD HTTP_HEAD_AWS
#define HTTP_POST HTTP_POST_AWS
#define HTTP_PUT HTTP_PUT_AWS
#define HTTP_PATCH HTTP_PATCH_AWS
#define HTTP_DELETE HTTP_DELETE_AWS
#define HTTP_OPTIONS HTTP_OPTIONS_AWS
#endif

#include <ESPAsyncWebServer.h>

#ifdef ESP8266
#undef HTTP_ANY
#endif

#if defined(ESP8266) || defined(ESP32)
#undef HTTP_GET
#undef HTTP_HEAD
#undef HTTP_POST
#undef HTTP_PUT
#undef HTTP_PATCH
#undef HTTP_DELETE
#undef HTTP_OPTIONS
#endif

#include "config_manager.h"
#include "metrics.h"

extern Config config;
extern Metrics metrics;
extern WiFiClient pool_client;
extern AsyncWebServer server;
extern AsyncServer stratum_server;
extern std::vector<AsyncClient*> connected_miners;

const char* GetBoardName();
