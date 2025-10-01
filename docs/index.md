# YUMA Stratum Proxy

A lightweight Stratum V1 proxy firmware for ESP32 and ESP8266 devices with an intuitive web interface.

## Overview

YUMA provides a seamless bridge between mining devices and upstream pools, featuring real-time monitoring, easy configuration, and automatic reconnection capabilities. Perfect for NerdMiner setups and small-scale mining operations.

## Key Features

- **🌐 Web Interface** - Configure everything through a responsive dashboard
- **📶 WiFi Manager** - Captive portal for easy first-time setup
- **⚡ Stratum V1** - Full proxy support with automatic difficulty adjustment
- **📊 Real-time Metrics** - Monitor shares, jobs, uptime and connection status
- **🔄 Auto-reconnection** - Automatic retry for WiFi and pool connections
- **📡 mDNS Support** - Access via `yuma.local` for easy discovery

## Supported Hardware

- **ESP32** development boards with WiFi
- **ESP8266** modules (ESP-WROOM-02) with ≥2MB flash
- Optional 128×64 OLED display support

## Quick Start

1. **Flash** the firmware to your ESP32/ESP8266
2. **Connect** to the `YUMA-PROXY` WiFi network (password: `12345678`)
3. **Configure** your WiFi credentials through the captive portal
4. **Access** the web interface at `http://[device-ip]`
5. **Setup** your mining pool configuration

## Web Interface Preview

The dashboard provides real-time monitoring of:
- Pool connection status
- Share statistics (accepted/rejected)
- Current difficulty and job information
- WiFi signal strength
- System uptime

## Getting Started

Ready to set up your YUMA proxy? Check out our comprehensive setup guide and documentation.

---

**⚡ Start mining smarter with YUMA! ⚡**