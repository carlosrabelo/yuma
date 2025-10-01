#!/bin/bash

# Firmware Assets Generation Script
# Builds all firmware variants and organizes them for web flasher

# Configuration
ASSETS_DIR="assets"
FIRMWARE_DIR="$ASSETS_DIR/firmware"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIO_CHECK="$SCRIPT_DIR/pio_check.sh"

# Get version from git
VERSION=$(git describe --tags --always --dirty 2>/dev/null || echo "v1.0.0-dev")
VERSION_CLEAN=$(echo "$VERSION" | sed 's/^v//')

# All environments for building
ALL_ENVS="esp32dev esp_wroom_02 esp32dev_oled esp_wroom_02_oled"

# Function to clean assets directory
clean_assets() {
    echo "🧹 Cleaning assets directory..."
    rm -rf "$ASSETS_DIR/firmware/"*
    sleep 0.1
    mkdir -p "$FIRMWARE_DIR/esp32"/{standard,oled} "$FIRMWARE_DIR/esp8266"/{standard,oled}
}

# Function to build and copy firmware for specific environment
build_and_copy() {
    local env="$1"
    local chip="$2"
    local variant="$3"

    echo "  📦 Building $env ($chip $variant)..."

    # Build the firmware
    if ! "$PIO_CHECK" run run --environment "$env" >/dev/null 2>&1; then
        echo "    ❌ Build failed for $env"
        return 1
    fi

    # Copy files to target directory
    local target_dir="$FIRMWARE_DIR/$chip/$variant"
    local build_path=".pio/build/$env"

    mkdir -p "$target_dir"

    # Copy main firmware binary
    if [ -f "$build_path/firmware.bin" ]; then
        cp "$build_path/firmware.bin" "$target_dir/yuma-$chip-$variant-$VERSION_CLEAN.bin"
        echo "    ✅ $target_dir/yuma-$chip-$variant-$VERSION_CLEAN.bin"
    else
        echo "    ❌ firmware.bin not found for $env"
        return 1
    fi

    # Copy bootloader for ESP32
    if [ "$chip" = "esp32" ] && [ -f "$build_path/bootloader.bin" ]; then
        cp "$build_path/bootloader.bin" "$target_dir/"
        echo "    ✅ $target_dir/bootloader.bin"
    fi

    return 0
}

# Function to build ESP32 variants
build_esp32() {
    echo "🔧 Building ESP32 firmware variants..."
    build_and_copy "esp32dev" "esp32" "standard"
    build_and_copy "esp32dev_oled" "esp32" "oled"
}

# Function to build ESP8266 variants
build_esp8266() {
    echo "🔧 Building ESP8266 firmware variants..."
    build_and_copy "esp_wroom_02" "esp8266" "standard"
    build_and_copy "esp_wroom_02_oled" "esp8266" "oled"
}

# Main execution
case "$1" in
    "clean")
        clean_assets
        ;;
    "esp32")
        build_esp32
        ;;
    "esp8266")
        build_esp8266
        ;;
    "all")
        echo "🏗️  Building all firmware variants for version $VERSION..."

        # Check PlatformIO first
        if ! "$PIO_CHECK" check; then
            exit 1
        fi

        clean_assets
        build_esp32
        build_esp8266

        # Generate manifest
        "$SCRIPT_DIR/generate_manifest.sh"

        echo "✅ All firmware binaries generated in $ASSETS_DIR/"
        ;;
    *)
        echo "Usage: $0 {clean|esp32|esp8266|all}"
        echo "  clean    - Clean assets directory"
        echo "  esp32    - Build ESP32 variants only"
        echo "  esp8266  - Build ESP8266 variants only"
        echo "  all      - Build all variants and generate manifest"
        exit 1
        ;;
esac