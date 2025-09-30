#include "web_interface.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <cstring>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "app_context.h"
#include "config_manager.h"
#include "wifi_setup.h"

void SetupWebServer() {
    if (server != nullptr) {
        delete server;
    }
    server = new AsyncWebServer(80);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>YUNA Stratum Proxy</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #1a1a1a; color: #00ff00; }
        .container { max-width: 800px; margin: 0 auto; }
        .status { background-color: #2a2a2a; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .metric { display: flex; justify-content: space-between; margin: 5px 0; }
        .config-form { background-color: #2a2a2a; padding: 20px; border-radius: 5px; margin: 20px 0; }
        input, select { background-color: #3a3a3a; color: #00ff00; border: 1px solid #555; padding: 8px; margin: 5px; border-radius: 3px; }
        button { background-color: #0066cc; color: white; border: none; padding: 10px 20px; border-radius: 3px; cursor: pointer; }
        button:hover { background-color: #0055aa; }
        .green { color: #00ff00; }
        .red { color: #ff4444; }
        .orange { color: #ffaa00; }
        h1, h2 { color: #00aaff; }
        .static-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 10px; }
        .static-grid div { display: flex; flex-direction: column; }
    </style>
    <script>
        function updateStats() {
            fetch('/api/status').then(r => r.json()).then(data => {
                document.getElementById('status').innerHTML =
                    '<div class="metric"><span>Pool Status:</span><span class="' + (data.pool_connected ? 'green">Connected' : 'red">Disconnected') + '</span></div>' +
                    '<div class="metric"><span>Uptime:</span><span>' + data.uptime + '</span></div>' +
                    '<div class="metric"><span>Shares OK:</span><span class="green">' + data.shares_ok + '</span></div>' +
                    '<div class="metric"><span>Shares Bad:</span><span class="red">' + data.shares_bad + '</span></div>' +
                    '<div class="metric"><span>Jobs Received:</span><span>' + data.jobs_received + '</span></div>' +
                    '<div class="metric"><span>Difficulty:</span><span>' + data.current_difficulty + '</span></div>' +
                    '<div class="metric"><span>Last Job:</span><span>' + data.last_job_id + '</span></div>' +
                    '<div class="metric"><span>Connected Miners:</span><span>' + data.connected_miners_count + '</span></div>' +
                    '<div class="metric"><span>IP Address:</span><span>' + data.ip_address + '</span></div>' +
                    '<div class="metric"><span>mDNS Address:</span><span><a href="http://yuma.local" target="_blank">yuma.local</a></span></div>' +
                    '<div class="metric"><span>Gateway:</span><span>' + data.gateway + '</span></div>' +
                    '<div class="metric"><span>Static IP Mode:</span><span class="' + (data.static_ip_mode ? 'green">Enabled' : 'orange">DHCP') + '</span></div>' +
                    '<div class="metric"><span>WiFi RSSI:</span><span>' + data.wifi_rssi + ' dBm</span></div>';
            });
        }
        setInterval(updateStats, 5000);
        window.onload = updateStats;
    </script>
</head>
<body>
    <div class="container">
        <h1>YUNA Stratum Proxy</h1>

        <div class="status">
            <h2>System Status</h2>
            <div id="status">Loading...</div>
        </div>

        <div class="config-form">
            <h2>Configuration</h2>
            <form action="/config" method="POST">
                <div>
                    <label>Pool Host:</label><br>
                    <input type="text" name="pool_host" value=")HTML" + String(config.pool_host) + R"HTML(" style="width: 300px;">
                </div>
                <div>
                    <label>Pool Port:</label><br>
                    <input type="number" name="pool_port" value=")HTML" + String(config.pool_port) + R"HTML(">
                </div>
                <div>
                    <label>Pool User (Wallet):</label><br>
                    <input type="text" name="pool_user" value=")HTML" + String(config.pool_user) + R"HTML(" style="width: 400px;">
                </div>
                <div>
                    <label>Pool Password:</label><br>
                    <input type="text" name="pool_pass" value=")HTML" + String(config.pool_pass) + R"HTML(">
                </div>
                <div>
                    <label>Initial Difficulty:</label><br>
                    <input type="number" name="difficulty" value=")HTML" + String(config.difficulty) + R"HTML(">
                </div>
                <div>
                    <input type="checkbox" name="vardiff_enabled" )HTML" + String(config.vardiff_enabled ? "checked" : "") + R"HTML(">
                    <label>VarDiff Enabled</label>
                </div>
                <div>
                    <input type="checkbox" name="use_static_ip" )HTML" + String(config.use_static_ip ? "checked" : "") + R"HTML(">
                    <label>Use Static IP</label>
                </div>
                <div class="static-grid">
                    <div>
                        <label>Static IP</label>
                        <input type="text" name="static_ip" value=")HTML" + String(config.static_ip) + R"HTML(" placeholder="192.168.1.50">
                    </div>
                    <div>
                        <label>Gateway</label>
                        <input type="text" name="static_gateway" value=")HTML" + String(config.static_gateway) + R"HTML(" placeholder="192.168.1.1">
                    </div>
                    <div>
                        <label>Subnet Mask</label>
                        <input type="text" name="static_subnet" value=")HTML" + String(config.static_subnet) + R"HTML(" placeholder="255.255.255.0">
                    </div>
                    <div>
                        <label>DNS</label>
                        <input type="text" name="static_dns" value=")HTML" + String(config.static_dns) + R"HTML(" placeholder="192.168.1.1">
                    </div>
                </div>
                <br>
                <button type="submit">Save Configuration</button>
                <button type="button" onclick="location.href='/restart'">Restart</button>
            </form>
        </div>

        <div class="status">
            <h2>Actions</h2>
            <button onclick="location.href='/reset_wifi'">Reset WiFi</button>
            <button onclick="location.href='/test_pool'">Test Pool</button>
        </div>
    </div>
</body>
</html>
        )HTML";
        request->send(200, "text/html", html);
    });

    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(512);

        unsigned long uptime_seconds = (millis() - metrics.uptime_start) / 1000;
        doc["pool_connected"] = metrics.pool_connected;
        doc["uptime"] = String(uptime_seconds / 3600) + "h " + String((uptime_seconds % 3600) / 60) + "m";
        doc["shares_ok"] = metrics.shares_ok;
        doc["shares_bad"] = metrics.shares_bad;
        doc["jobs_received"] = metrics.jobs_received;
        doc["current_difficulty"] = metrics.current_difficulty;
        doc["last_job_id"] = metrics.last_job_id;
        doc["connected_miners_count"] = connected_miners.size();
        doc["wifi_rssi"] = WiFi.RSSI();
        doc["ip_address"] = WiFi.localIP().toString();
        doc["gateway"] = WiFi.gatewayIP().toString();
        doc["static_ip_mode"] = config.use_static_ip;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server->on("/config", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("pool_host", true)) {
            CopyStringField(config.pool_host, sizeof(config.pool_host), request->getParam("pool_host", true)->value());
        }
        if (request->hasParam("pool_port", true)) {
            config.pool_port = request->getParam("pool_port", true)->value().toInt();
        }
        if (request->hasParam("pool_user", true)) {
            CopyStringField(config.pool_user, sizeof(config.pool_user), request->getParam("pool_user", true)->value());
        }
        if (request->hasParam("pool_pass", true)) {
            CopyStringField(config.pool_pass, sizeof(config.pool_pass), request->getParam("pool_pass", true)->value());
        }
        if (request->hasParam("difficulty", true)) {
            config.difficulty = request->getParam("difficulty", true)->value().toInt();
        }
        config.vardiff_enabled = request->hasParam("vardiff_enabled", true);
        bool static_requested = request->hasParam("use_static_ip", true);

        if (request->hasParam("static_ip", true)) {
            CopyStringField(config.static_ip, sizeof(config.static_ip), request->getParam("static_ip", true)->value());
        }
        if (request->hasParam("static_gateway", true)) {
            CopyStringField(config.static_gateway, sizeof(config.static_gateway), request->getParam("static_gateway", true)->value());
        }
        if (request->hasParam("static_subnet", true)) {
            CopyStringField(config.static_subnet, sizeof(config.static_subnet), request->getParam("static_subnet", true)->value());
        }
        if (request->hasParam("static_dns", true)) {
            CopyStringField(config.static_dns, sizeof(config.static_dns), request->getParam("static_dns", true)->value());
        }

        if (static_requested) {
            IPAddress ip;
            IPAddress gateway;
            IPAddress subnet;
            IPAddress dns;

            bool ip_ok = ip.fromString(config.static_ip);
            bool gateway_ok = gateway.fromString(config.static_gateway);
            bool subnet_ok = subnet.fromString(config.static_subnet);
            bool dns_ok = (strlen(config.static_dns) == 0) || dns.fromString(config.static_dns);

            if (!(ip_ok && gateway_ok && subnet_ok && dns_ok)) {
                request->send(400, "text/plain", "Invalid static IP configuration");
                return;
            }
        }

        config.use_static_ip = static_requested;

        if (!SaveConfig(config)) {
            request->send(500, "text/plain", "Failed to save configuration");
            return;
        }

        request->redirect("/");
    });

    server->on("/restart", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart();
    });

    server->on("/reset_wifi", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Resetting WiFi...");
        ResetWifiSettings();
        delay(1000);
        ESP.restart();
    });

    server->on("/test_pool", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Pool test not implemented yet");
    });

    server->begin();
    Serial.println("Web server started");
}
