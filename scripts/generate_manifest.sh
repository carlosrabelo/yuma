#!/bin/bash

# Manifest.json Generation Script
# Creates manifest file for ESP Web Tools flasher

# Configuration
ASSETS_DIR="assets"
FIRMWARE_DIR="$ASSETS_DIR/firmware"
MANIFEST_FILE="$ASSETS_DIR/manifest.json"

# Get version from git
VERSION=$(git describe --tags --always --dirty 2>/dev/null || echo "v1.0.0-dev")
VERSION_CLEAN=$(echo "$VERSION" | sed 's/^v//')

# Function to generate manifest.json
generate_manifest() {
    echo "📋 Generating manifest.json..."

    # Start JSON structure
    cat > "$MANIFEST_FILE" << EOF
{
  "name": "YUMA Stratum Proxy",
  "version": "$VERSION_CLEAN",
  "builds": [
EOF

    local first_entry=true

    # Process each chip and variant
    for chip in esp32 esp8266; do
        for variant in standard oled; do
            local firmware_file="$FIRMWARE_DIR/$chip/$variant/yuma-$chip-$variant-$VERSION_CLEAN.bin"

            if [ -f "$firmware_file" ]; then
                # Add comma separator for non-first entries
                if [ "$first_entry" = false ]; then
                    echo "    ," >> "$MANIFEST_FILE"
                fi
                first_entry=false

                # Set appropriate offsets for chip type
                local firmware_offset
                local bootloader_offset
                if [ "$chip" = "esp32" ]; then
                    firmware_offset=65536
                    bootloader_offset=4096
                else
                    firmware_offset=0
                fi

                # Add build entry
                cat >> "$MANIFEST_FILE" << EOF
    {
      "chipFamily": "$chip",
      "variant": "$variant",
      "parts": [
EOF

                # Add bootloader first for ESP32 (if exists)
                if [ "$chip" = "esp32" ] && [ -f "$FIRMWARE_DIR/$chip/$variant/bootloader.bin" ]; then
                    cat >> "$MANIFEST_FILE" << EOF
        {
          "path": "firmware/$chip/$variant/bootloader.bin",
          "offset": $bootloader_offset
        },
EOF
                fi

                # Add main firmware
                cat >> "$MANIFEST_FILE" << EOF
        {
          "path": "firmware/$chip/$variant/yuma-$chip-$variant-$VERSION_CLEAN.bin",
          "offset": $firmware_offset
        }
EOF

                # Close parts array and build object
                echo "      ]" >> "$MANIFEST_FILE"
                echo "    }" >> "$MANIFEST_FILE"
            fi
        done
    done

    # Close JSON structure
    cat >> "$MANIFEST_FILE" << EOF
  ]
}
EOF

    echo "✅ Generated $MANIFEST_FILE"
}

# Function to validate manifest
validate_manifest() {
    if command -v python3 >/dev/null 2>&1; then
        if python3 -m json.tool "$MANIFEST_FILE" >/dev/null 2>&1; then
            echo "✅ Manifest JSON is valid"
        else
            echo "❌ Manifest JSON is invalid"
            return 1
        fi
    else
        echo "⚠️  Python3 not available for JSON validation"
    fi
}

# Function to show manifest info
show_info() {
    if [ -f "$MANIFEST_FILE" ]; then
        echo "📋 Manifest information:"
        echo "  File: $MANIFEST_FILE"
        echo "  Size: $(wc -c < "$MANIFEST_FILE") bytes"

        if command -v python3 >/dev/null 2>&1; then
            local builds_count=$(python3 -c "
import json
with open('$MANIFEST_FILE') as f:
    data = json.load(f)
    print(len(data['builds']))
" 2>/dev/null || echo "unknown")
            echo "  Builds: $builds_count"
        fi
    else
        echo "❌ Manifest file not found: $MANIFEST_FILE"
        return 1
    fi
}

# Main execution
case "$1" in
    "generate")
        generate_manifest
        validate_manifest
        ;;
    "validate")
        validate_manifest
        ;;
    "info")
        show_info
        ;;
    *)
        echo "Usage: $0 {generate|validate|info}"
        echo "  generate  - Generate manifest.json from firmware files"
        echo "  validate  - Validate existing manifest.json"
        echo "  info      - Show manifest information"
        exit 1
        ;;
esac