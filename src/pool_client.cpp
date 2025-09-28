#include "pool_client.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif

#include "app_context.h"

namespace {
String extranonce1 = "";
int extranonce2_size = 4;
bool subscribed = false;
bool authorized = false;

String SanitizePoolHost(const char* raw_host) {
    String host = String(raw_host);
    host.trim();

    int schemeIdx = host.indexOf("://");
    if (schemeIdx != -1) {
        host = host.substring(schemeIdx + 3);
    }

    int slashIdx = host.indexOf('/');
    if (slashIdx != -1) {
        host = host.substring(0, slashIdx);
    }

    int colonIdx = host.indexOf(':');
    if (colonIdx != -1) {
        host = host.substring(0, colonIdx);
    }

    if (host.length() == 0) {
        host = String(raw_host);
    }

    return host;
}
}

void ConnectToPool() {
    String host = SanitizePoolHost(config.pool_host);
    Serial.printf("Connecting to pool %s:%d\n", host.c_str(), config.pool_port);

    if (pool_client.connect(host.c_str(), config.pool_port)) {
        Serial.println("Connected to pool!");
        metrics.pool_connected = true;

        DynamicJsonDocument doc(256);
        doc["id"] = 1;
        doc["method"] = "mining.subscribe";
        doc["params"][0] = "ESPStratumProxy/1.0";

        String message;
        serializeJson(doc, message);
        message += "\n";

        pool_client.print(message);
        Serial.println("Subscribe sent");
    } else {
        Serial.println("Failed to connect to pool");
        metrics.pool_connected = false;
        delay(30000);
    }
}

void HandlePoolData() {
    if (!pool_client.available()) {
        return;
    }

    String line = pool_client.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
        return;
    }

    Serial.println("Pool: " + line);

    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, line) != DeserializationError::Ok) {
        return;
    }

    if (doc.containsKey("method")) {
        String method = doc["method"];

        if (method == "mining.set_difficulty") {
            if (doc["params"].is<JsonArray>() && doc["params"].size() > 0) {
                metrics.current_difficulty = doc["params"][0];
                Serial.printf("New difficulty: %.2f\n", metrics.current_difficulty);
            }
        } else if (method == "mining.notify") {
            if (doc["params"].is<JsonArray>() && doc["params"].size() > 0) {
                metrics.last_job_id = doc["params"][0].as<String>();
                metrics.jobs_received++;
                Serial.println("New job received: " + metrics.last_job_id);
            }
        }
    } else if (doc.containsKey("result")) {
        int id = doc["id"] | 0;

        if (id == 1) {
            if (doc["result"].is<JsonArray>() && doc["result"].size() >= 3) {
                extranonce1 = doc["result"][1].as<String>();
                extranonce2_size = doc["result"][2];
                subscribed = true;
                Serial.println("Subscribe OK, sending authorize");

                DynamicJsonDocument auth_doc(256);
                auth_doc["id"] = 2;
                auth_doc["method"] = "mining.authorize";
                auth_doc["params"][0] = config.pool_user;
                auth_doc["params"][1] = config.pool_pass;

                String auth_message;
                serializeJson(auth_doc, auth_message);
                auth_message += "\n";

                pool_client.print(auth_message);
            }
        } else if (id == 2) {
            if (doc["result"].as<bool>()) {
                authorized = true;
                Serial.println("Authorized OK");
            } else {
                Serial.println("Authorization failed");
            }
        } else if (id >= 1000) {
            if (doc["result"].as<bool>()) {
                metrics.shares_ok++;
                metrics.last_share_time = millis();
                Serial.println("Share accepted!");
            } else {
                metrics.shares_bad++;
                Serial.println("Share rejected!");
                if (doc.containsKey("error") && doc["error"].size() > 1) {
                    Serial.println("Error: " + doc["error"][1].as<String>());
                }
            }
        }
    }
}

void DisconnectFromPool() {
    if (pool_client.connected()) {
        pool_client.stop();
        metrics.pool_connected = false;
        subscribed = false;
        authorized = false;
        Serial.println("Disconnected from pool (no miners connected)");
    }
}

bool ShouldConnectToPool() {
    return connected_miners.size() > 0;
}
