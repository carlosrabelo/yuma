#!/bin/bash
# ESP Board Auto Detection Script
# Detects ESP32/ESP8266 boards connected via USB and updates PlatformIO configuration

PLATFORMIO_INI="platformio.ini"
PLATFORMIO_TEMPLATE="platformio.ini.template"
MAKEFILE_TEMPLATE="Makefile.template"

# Ensure config files exist (copy from templates if needed)
if [ ! -f "$PLATFORMIO_INI" ] && [ -f "$PLATFORMIO_TEMPLATE" ]; then
    echo "📋 Creating platformio.ini from template..."
    cp "$PLATFORMIO_TEMPLATE" "$PLATFORMIO_INI"
fi

if [ ! -f "Makefile" ] && [ -f "$MAKEFILE_TEMPLATE" ]; then
    echo "📋 Creating Makefile from template..."
    cp "$MAKEFILE_TEMPLATE" "Makefile"
fi

echo "🔍 Searching for ESP boards (ESP32/ESP8266)..."

# Function to detect board type using esptool
detect_board_type() {
    local port="$1"
    echo "🔎 Probing device at $port..." >&2

    # Find esptool.py in common locations
    local esptool_paths=(
        "esptool.py"
        "~/.platformio/packages/tool-esptoolpy/esptool.py"
        "~/.local/bin/esptool.py"
    )

    local esptool_cmd=""
    for path in "${esptool_paths[@]}"; do
        expanded_path=$(eval echo "$path")
        if command -v "$path" >/dev/null 2>&1 || [ -f "$expanded_path" ]; then
            if command -v "$path" >/dev/null 2>&1; then
                esptool_cmd="$path"
            else
                esptool_cmd="python3 $expanded_path"
            fi
            break
        fi
    done

    # Try to detect using esptool if found
    if [ -n "$esptool_cmd" ]; then
        # Use esptool to identify the chip
        CHIP_INFO=$(timeout 10 $esptool_cmd --port "$port" chip_id 2>/dev/null | grep "Chip is")
        if [ $? -eq 0 ] && [ -n "$CHIP_INFO" ]; then
            if echo "$CHIP_INFO" | grep -qi "ESP32"; then
                echo "esp32"
                return 0
            elif echo "$CHIP_INFO" | grep -qi "ESP8266"; then
                echo "esp8266"
                return 0
            fi
        fi
    fi

    # Fallback: analyze USB vendor/product info
    USB_INFO=$(udevadm info --name="$port" 2>/dev/null | grep -E "(ID_VENDOR|ID_MODEL|ID_SERIAL)")

    # ESP32 indicators
    if echo "$USB_INFO" | grep -qi -E "(espressif|esp32|0403:6001.*esp32|10c4:ea60.*esp32)"; then
        echo "esp32"
        return 0
    fi

    # ESP8266 indicators (often use CH340, CP210x, or FTDI chips)
    if echo "$USB_INFO" | grep -qi -E "(ch340|cp210|wch\.cn|1a86:7523)"; then
        echo "esp8266"
        return 0
    fi

    # FTDI-based boards (could be either, default to ESP32)
    if echo "$USB_INFO" | grep -qi -E "(ftdi|0403:6001)"; then
        echo "esp32"
        return 0
    fi

    # Silicon Labs CP210x (common with ESP32)
    if echo "$USB_INFO" | grep -qi -E "(silicon.*labs|10c4:ea60)"; then
        echo "esp32"
        return 0
    fi

    # Default fallback
    echo "unknown"
    return 1
}

# Structure to hold device information
declare -A DEVICE_PORTS
declare -A DEVICE_TYPES
declare -A DEVICE_INFO

# Look for ESP32/ESP8266 devices in common locations
ESP_PORTS=""

# Check /dev/ttyUSB* ports
for port in /dev/ttyUSB*; do
    if [ -e "$port" ] && [ -r "$port" ] && [ -w "$port" ]; then
        # Get basic USB info
        USB_INFO=$(udevadm info --name="$port" 2>/dev/null | grep -E "(ID_VENDOR|ID_MODEL|ID_SERIAL)" | head -3)

        # Check if it's likely an ESP32/ESP8266 by looking at USB vendor/product info
        if echo "$USB_INFO" | grep -qi -E "(espressif|silicon.*labs|cp210|ch340|ftdi|wch\.cn|1a86:7523|10c4:ea60|0403:6001)" || [ -z "$USB_INFO" ]; then
            ESP_PORTS="$ESP_PORTS $port"
            DEVICE_PORTS["$port"]="$port"
            DEVICE_INFO["$port"]="$USB_INFO"

            echo "  📍 Found potential ESP device at: $port"
            if [ -n "$USB_INFO" ]; then
                echo "$USB_INFO" | sed 's/^/    /'
            fi

            # Detect board type
            BOARD_TYPE=$(detect_board_type "$port")
            DEVICE_TYPES["$port"]="$BOARD_TYPE"

            if [ "$BOARD_TYPE" != "unknown" ]; then
                echo "    🎯 Detected: $(echo $BOARD_TYPE | tr '[:lower:]' '[:upper:]')"
            else
                echo "    ❓ Board type: Unknown (will default to ESP32)"
            fi
            echo ""
        fi
    fi
done

# Check /dev/ttyACM* ports (Arduino-style devices)
for port in /dev/ttyACM*; do
    if [ -e "$port" ] && [ -r "$port" ] && [ -w "$port" ]; then
        ESP_PORTS="$ESP_PORTS $port"
        DEVICE_PORTS["$port"]="$port"
        DEVICE_INFO["$port"]="Arduino-compatible device"

        echo "  📍 Found Arduino-style device at: $port"

        # Detect board type
        BOARD_TYPE=$(detect_board_type "$port")
        DEVICE_TYPES["$port"]="$BOARD_TYPE"

        if [ "$BOARD_TYPE" != "unknown" ]; then
            echo "    🎯 Detected: $(echo $BOARD_TYPE | tr '[:lower:]' '[:upper:]')"
        else
            echo "    ❓ Board type: Unknown (will default to ESP32)"
        fi
        echo ""
    fi
done

# Remove leading space
ESP_PORTS=$(echo "$ESP_PORTS" | sed 's/^ //')

if [ -z "$ESP_PORTS" ]; then
    echo "❌ No ESP32/ESP8266 devices found!"
    echo "   Make sure your device is connected and the USB cable supports data transfer."
    exit 1
fi

# Convert to array
PORT_ARRAY=($ESP_PORTS)

if [ ${#PORT_ARRAY[@]} -eq 1 ]; then
    SELECTED_PORT=${PORT_ARRAY[0]}
    SELECTED_TYPE=${DEVICE_TYPES["$SELECTED_PORT"]}
    echo "✅ Auto-selected: $SELECTED_PORT ($(echo $SELECTED_TYPE | tr '[:lower:]' '[:upper:]'))"
else
    echo ""
    echo "Multiple devices found:"
    for i in "${!PORT_ARRAY[@]}"; do
        port=${PORT_ARRAY[$i]}
        board_type=${DEVICE_TYPES["$port"]}
        echo "  $((i+1)). $port - $(echo $board_type | tr '[:lower:]' '[:upper:]')"
    done
    echo ""
    read -p "Select device (1-${#PORT_ARRAY[@]}): " choice

    if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le ${#PORT_ARRAY[@]} ]; then
        SELECTED_PORT=${PORT_ARRAY[$((choice-1))]}
        SELECTED_TYPE=${DEVICE_TYPES["$SELECTED_PORT"]}
        echo "✅ Selected: $SELECTED_PORT ($(echo $SELECTED_TYPE | tr '[:lower:]' '[:upper:]'))"
    else
        echo "❌ Invalid selection!"
        exit 1
    fi
fi

# Determine the appropriate PlatformIO environment based on detected board type
case "$SELECTED_TYPE" in
    "esp32")
        RECOMMENDED_ENV="esp32dev"
        DEFAULT_ENVS="esp32dev"
        echo "🎯 Recommended environment: $RECOMMENDED_ENV"
        ;;
    "esp8266")
        RECOMMENDED_ENV="esp_wroom_02"
        DEFAULT_ENVS="esp_wroom_02"
        echo "🎯 Recommended environment: $RECOMMENDED_ENV"
        ;;
    *)
        RECOMMENDED_ENV="esp32dev"
        DEFAULT_ENVS="esp32dev, esp_wroom_02"
        echo "❓ Unknown board type, defaulting to: $RECOMMENDED_ENV"
        ;;
esac

# Update platformio.ini configuration
echo "📝 Updating configuration files..."

# Backup the current platformio.ini
cp "$PLATFORMIO_INI" "$PLATFORMIO_INI.bak"

# Update default environments based on detected board type
sed -i \
    -e "s|^default_envs.*|default_envs = $DEFAULT_ENVS|g" \
    "$PLATFORMIO_INI"

# Update all environment sections with the selected port
sed -i \
    -e "s|^[[:space:]]*upload_port.*|upload_port = $SELECTED_PORT|g" \
    -e "s|^[[:space:]]*monitor_port.*|monitor_port = $SELECTED_PORT|g" \
    "$PLATFORMIO_INI"

echo "✅ Updated $PLATFORMIO_INI with:"
echo "   • Default environment: $DEFAULT_ENVS"
echo "   • Upload/Monitor port: $SELECTED_PORT"
echo ""

# Update Makefile default board if needed
if [ -f "Makefile" ]; then
    case "$SELECTED_TYPE" in
        "esp32")
            MAKEFILE_BOARD="esp32"
            ;;
        "esp8266")
            MAKEFILE_BOARD="esp8266"
            ;;
        *)
            MAKEFILE_BOARD="esp32"
            ;;
    esac

    # Update Makefile BOARD default and ports
    sed -i.bak \
        -e "s|^BOARD\s*?=.*|BOARD\t\t?= $MAKEFILE_BOARD|g" \
        -e "s|^MONITOR_PORT\s*=.*|MONITOR_PORT\t= $SELECTED_PORT|g" \
        -e "s|^UPLOAD_PORT\s*=.*|UPLOAD_PORT\t= $SELECTED_PORT|g" \
        Makefile
    echo "✅ Updated Makefile default BOARD to: $MAKEFILE_BOARD"
    echo "✅ Updated Makefile ports to: $SELECTED_PORT"
fi

echo ""
echo "🎉 ESP board configuration completed successfully!"
echo ""
echo "📋 Device Summary:"
echo "   Port: $SELECTED_PORT"
echo "   Type: $(echo $SELECTED_TYPE | tr '[:lower:]' '[:upper:]')"
echo "   Environment: $RECOMMENDED_ENV"
echo ""
echo "🔧 Available commands:"
echo "   make build      # Compile for detected board ($SELECTED_TYPE)"
echo "   make upload     # Upload to $SELECTED_PORT"
echo "   make monitor    # Start serial monitor"
echo "   make flash      # Build + upload in one step"
echo "   make clean      # Clean build files"
echo ""
echo "🎯 Board-specific commands:"
if [ "$SELECTED_TYPE" = "esp32" ]; then
    echo "   make build BOARD=esp32       # ESP32 standard"
    echo "   make build BOARD=esp32_oled  # ESP32 with OLED"
elif [ "$SELECTED_TYPE" = "esp8266" ]; then
    echo "   make build BOARD=esp8266       # ESP8266 standard"
    echo "   make build BOARD=esp8266_oled  # ESP8266 with OLED"
else
    echo "   make build BOARD=esp32         # ESP32 (default)"
    echo "   make build BOARD=esp8266       # ESP8266"
    echo "   make build BOARD=esp32_oled    # ESP32 with OLED"
    echo "   make build BOARD=esp8266_oled  # ESP8266 with OLED"
fi
echo ""
echo "💡 Tip: Run 'make help' to see all available options"