# ESP Stratum Proxy

Stratum V1 proxy firmware with an intuitive web interface that now runs on both ESP-WROOM modules (ESP8266) and ESP32 development boards. Configure Wi-Fi, mining pools, and monitor real-time metrics directly from the device.

## 🚀 Features

- **Intuitive Web Interface**: configure everything from a responsive dashboard
- **WiFi Manager**: captive portal for first-time provisioning
- **Stratum V1**: full proxy between miners and upstream pool
- **Real-time Metrics**: shares, jobs, uptime, RSSI, and miner count
- **VarDiff Settings**: optional automatic difficulty targets
- **Persistent Configuration**: LittleFS/SPIFFS-backed JSON configuration storage
- **Auto-reconnection**: retries Wi-Fi and pool links automatically

## 🔧 Supported Hardware

- **ESP-WROOM-02 (ESP8266)** modules or boards with ≥2 MB flash (4 MB recommended for SPIFFS)
- **ESP-WROOM-32 / ESP32** development boards with Wi-Fi
- Stable 5 V power supply (1 A recommended for reliability)

## 📦 Installation

### Via PlatformIO

```bash
# Clone the repository
git clone <repo-url>
cd yuma

# Install dependencies once
pio pkg install

# Build & flash the ESP-WROOM-02 (default environment)
pio run -e esp_wroom_02 --target upload

# Optional: build & flash an ESP32 dev board
pio run -e esp32dev --target upload

# Monitor serial output (set the right port if needed)
pio device monitor

# Auto-build & upload (Makefile helper)
make upload  # picks ESP8266/ESP32 automatically based on the connected board
```

### Via Arduino IDE

1. Install required libraries:
   - WiFiManager
   - ArduinoJson
   - ESPAsyncWebServer
   - **ESP8266**: ESPAsyncTCP
   - **ESP32**: AsyncTCP
2. Open `src/main.cpp` in the IDE
3. Select the correct board (`ESP8266 Generic Module`/`ESP-WROOM-02` **or** `ESP32 Dev Module`)
4. Upload the sketch and open the serial monitor at 115200 baud

## 🌐 Initial Setup

1. Flash the firmware and reset the board
2. Join the temporary Wi-Fi network `YUNA-StratumProxy` (password `12345678`)
3. Follow the captive-portal wizard to store your Wi-Fi credentials
4. After the board connects, browse to `http://<device-ip>`

## 🛠️ Pool & Network Configuration

1. Connect a browser to the dashboard at `http://<device-ip>`
2. Fill in the form:
   - **Pool Host**: hostname or full stratum URI (e.g. `stratum+tcp://pool.example.com`)
   - **Pool Port**: upstream port (default `4444`)
   - **Pool User**: wallet or worker name
   - **Pool Password**: worker password (often `x`)
   - **Initial Difficulty** & **VarDiff** toggle (optional)
   - **Use Static IP**: tick to force a fixed address and provide:
     - **Static IP** (device address)
     - **Gateway**
     - **Subnet Mask**
     - **DNS** (fallbacks to gateway if blank)
3. Click **Save Configuration**. The proxy will persist the settings to flash and reload them on boot.
4. Use the **Restart** button in the Actions panel (or power-cycle) so the new network settings take effect.

> Static IP validation is done on save. If any field is invalid the firmware will fall back to DHCP, so double-check the values before rebooting.

### Optional OLED Status Display

If you attach a 128×64 I²C SSD1306, build with the OLED environment to mirror the live status on the screen:

```bash
# ESP32 target with display
pio run -e esp32dev_oled --target upload

# ESP8266 target with display
pio run -e esp_wroom_02_oled --target upload

# Makefile helper
make BOARD=esp32_oled flash
```

Wiring defaults:

| Signal | Pin |
| ------ | --- |
| SDA    | 2   |
| SCL    | 14  |
| VCC    | 3V3 |
| GND    | GND |

The OLED view refreshes every second showing IP address, pool status, connected miners, share counts, and difficulty.

## 📊 Web Interface

```
🚀 ESP Stratum Proxy
┌─────────────────────────────┐
│ 📊 System Status            │
│ Pool Status: Connected ✅   │
│ Uptime: 2h 15m             │
│ Shares OK: 45 ✅           │
│ Shares Bad: 3 ❌           │
│ Jobs Received: 156         │
│ Difficulty: 2048.00        │
│ Last Job: a1b2c3...        │
│ WiFi RSSI: -45 dBm         │
└─────────────────────────────┘
```

### API Endpoints

- `GET /` – dashboard HTML
- `GET /api/status` – live metrics in JSON
- `POST /config` – persist pool configuration
- `GET /restart` – soft reboot the device
- `GET /reset_wifi` – clear Wi-Fi credentials and reboot
- `GET /test_pool` – (placeholder) pool connectivity hook

## 🔍 Debugging

```
=== YUNA Stratum Proxy ===
WiFi connected!
IP: 192.168.1.100
Web server started
Connecting to pool pool.example.com:4444
Connected to pool!
Subscribe sent
Subscribe OK, sending authorize
Authorized OK
New job received: a1b2c3d4
```

### Status Codes

- ✅ **Green**: Connected / operating normally
- 🟡 **Yellow**: Waiting or retrying a link
- ❌ **Red**: Error state (Wi-Fi or pool disconnected)

## 🚨 Troubleshooting

### Wi-Fi fails to connect

Visit `http://192.168.4.1/reset_wifi` from a device connected to the access-point mode portal to clear saved Wi-Fi credentials.

### Pool connection issues

1. Double-check host/port (protocol prefix is optional)
2. Test connectivity from another device (`telnet pool.example.com 4444`)
3. Verify firewalls/proxies allow outbound TCP

### Shares rejected

1. Confirm wallet / worker credentials
2. Adjust difficulty or disable VarDiff
3. Check network latency and Wi-Fi RSSI (>-70 dBm recommended)

### Performance tips

1. Keep supply voltage stable (5 V / 1 A)
2. Reduce captive portal timeout if device is unattended
3. For ESP32 targets, consider core affinity & PSRAM for advanced setups

## 🎯 Next Steps

- Optimize networking stack for higher concurrency
- Add TLS support for secure pools
- Integrate OLED / display feedback
- Monitor board temperature and throttle if needed

## 📄 License

MIT License – see [LICENSE](LICENSE) for details.

---

**⚡ Happy Proxying! ⚡**
