#pragma once

#if defined(ESP32)
#include <SPIFFS.h>
#define STORAGE_FS SPIFFS
#elif defined(ESP8266)
#include <FS.h>
#include <LittleFS.h>
#define STORAGE_FS LittleFS
#else
#error "Unsupported board: STORAGE_FS not defined"
#endif
