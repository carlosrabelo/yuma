#include "stratum_server.h"

#include <Arduino.h>
#include <algorithm>

#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif

#include "app_context.h"
#include "pool_client.h"

void SetupStratumServer() {
    stratum_server.onClient([](void* arg, AsyncClient* client) {
        Serial.printf("New miner connected from %s\n", client->remoteIP().toString().c_str());

        connected_miners.push_back(client);
        metrics.connected_miners_count = connected_miners.size();

        client->onDisconnect([](void* arg, AsyncClient* client) {
            Serial.printf("Miner disconnected from %s\n", client->remoteIP().toString().c_str());

            auto it = std::find(connected_miners.begin(), connected_miners.end(), client);
            if (it != connected_miners.end()) {
                connected_miners.erase(it);
            }
            metrics.connected_miners_count = connected_miners.size();
        }, nullptr);

        client->onData([](void* arg, AsyncClient* client, void* data, size_t len) {
            String message = String(reinterpret_cast<const char*>(data)).substring(0, len);
            Serial.println("Miner data: " + message);
            if (pool_client.connected()) {
                pool_client.write(reinterpret_cast<const uint8_t*>(data), len);
            }
        }, nullptr);

    }, nullptr);

    stratum_server.begin();
    Serial.println("Stratum server started on port 4444");
}

void HandleMinerConnections() {
    // Future hook for additional miner management logic
}
