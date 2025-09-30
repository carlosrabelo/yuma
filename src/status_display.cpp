#include "status_display.h"

#ifdef USE_OLED_STATUS

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "app_context.h"

namespace {
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t SDA_PIN = 2;
constexpr uint8_t SCL_PIN = 14;
constexpr uint8_t TEXT_SIZE_SMALL = 1;
constexpr uint8_t TEXT_SIZE_LARGE = 2;

Adafruit_SSD1306 status_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
bool status_display_ready = false;
unsigned long last_display_update_ms = 0;
}

void SetupStatusDisplay() {
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!status_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Failed to initialize SSD1306 display");
        status_display_ready = false;
        return;
    }

    status_display_ready = true;
    status_display.clearDisplay();
    status_display.setTextColor(SSD1306_WHITE);
    status_display.setTextWrap(false);
    status_display.setTextSize(TEXT_SIZE_SMALL);
    status_display.setCursor(0, 0);
    status_display.println("YUMA Stratum Proxy");
    status_display.print("Board: ");
    status_display.println(GetBoardName());
    status_display.display();
}

void UpdateStatusDisplay() {
    if (!status_display_ready) {
        return;
    }

    unsigned long now = millis();
    if (now - last_display_update_ms < 1000) {
        return;
    }
    last_display_update_ms = now;

    status_display.clearDisplay();
    status_display.setTextColor(SSD1306_WHITE);
    status_display.setCursor(0, 0);

    status_display.setTextSize(TEXT_SIZE_LARGE);
    status_display.println("YUNA Proxy");

    status_display.setTextSize(TEXT_SIZE_SMALL);
    if (WiFi.status() == WL_CONNECTED) {
        status_display.print("IP: ");
        status_display.println(WiFi.localIP());
    } else {
        status_display.println("WiFi: offline");
    }

    status_display.print("Pool: ");
    status_display.println(metrics.pool_connected ? "up" : "down");

    status_display.print("Miners: ");
    status_display.println(connected_miners.size());

    status_display.print("Shares ");
    status_display.print(metrics.shares_ok);
    status_display.print('/');
    status_display.println(metrics.shares_bad);

    status_display.print("Diff: ");
    status_display.println(metrics.current_difficulty, 0);

    status_display.display();
}

#else

void SetupStatusDisplay() {}
void UpdateStatusDisplay() {}

#endif
