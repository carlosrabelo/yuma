#include "storage.h"

#include <Arduino.h>

#include "platform_fs.h"

namespace {
bool g_storage_mounted = false;
}

#if defined(ESP32)
static bool MountStorage(bool format_on_fail) {
    if (STORAGE_FS.begin(format_on_fail)) {
        g_storage_mounted = true;
        return true;
    }
    return false;
}
#elif defined(ESP8266)
static bool MountStorage(bool format_on_fail) {
    (void)format_on_fail;
    if (STORAGE_FS.begin()) {
        g_storage_mounted = true;
        return true;
    }
    if (format_on_fail && STORAGE_FS.format() && STORAGE_FS.begin()) {
        g_storage_mounted = true;
        return true;
    }
    return false;
}
#endif

bool SetupStorage() {
#if defined(ESP8266)
    if (!MountStorage(false)) {
        Serial.println("Error mounting LittleFS, attempting to format...");
        if (MountStorage(true)) {
            Serial.println("LittleFS formatted and mounted");
            return true;
        }
        Serial.println("LittleFS mount failed");
        return false;
    }
    Serial.println("LittleFS mounted");
    return true;
#else
    if (!MountStorage(false) && !MountStorage(true)) {
        Serial.println("Error mounting SPIFFS");
        return false;
    }
    Serial.println("SPIFFS mounted");
    return true;
#endif
}

bool EnsureStorageMounted() {
    if (g_storage_mounted) {
        return true;
    }

#if defined(ESP8266)
    if (MountStorage(false)) {
        Serial.println("LittleFS remounted");
        return true;
    }
    Serial.println("LittleFS remount failed");
    return false;
#else
    if (MountStorage(false) || MountStorage(true)) {
        Serial.println("SPIFFS remounted");
        return true;
    }
    Serial.println("SPIFFS remount failed");
    return false;
#endif
}
