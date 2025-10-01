# YUMA Stratum Proxy Makefile
# Build automation for PlatformIO projects

# Board/target selection
SUPPORTED_BOARDS:=esp8266 esp32 esp8266_oled esp32_oled
BOARD		?= esp32

ifeq ($(BOARD),esp32)
BUILD_ENV	:= esp32dev
AUTO_UPLOAD_ENVS:=esp32dev esp_wroom_02
else ifeq ($(BOARD),esp8266)
BUILD_ENV	:= esp_wroom_02
AUTO_UPLOAD_ENVS:=esp_wroom_02 esp32dev
else ifeq ($(BOARD),esp32_oled)
BUILD_ENV	:= esp32dev_oled
AUTO_UPLOAD_ENVS:=esp32dev_oled esp32dev esp_wroom_02_oled esp_wroom_02
else ifeq ($(BOARD),esp8266_oled)
BUILD_ENV	:= esp_wroom_02_oled
AUTO_UPLOAD_ENVS:=esp_wroom_02_oled esp_wroom_02 esp32dev_oled esp32dev
else
$(error Unsupported BOARD '$(BOARD)'. Supported values: $(SUPPORTED_BOARDS))
endif

UPLOAD_ENV_ARGS:=$(foreach env,$(AUTO_UPLOAD_ENVS),--env $(env))

# Variables
PLATFORM	= platformio
SRC_DIR		= src
BUILD_DIR	= .pio/build/$(BUILD_ENV)
ASSETS_DIR	= assets
FIRMWARE_DIR	= $(ASSETS_DIR)/firmware
MONITOR_PORT	= /dev/ttyUSB1
UPLOAD_PORT	= /dev/ttyUSB1
MONITOR_SPEED	= 115200

# Version extraction from git
VERSION		:= $(shell git describe --tags --always --dirty 2>/dev/null || echo "v1.0.0-dev")
VERSION_CLEAN	:= $(shell echo $(VERSION) | sed 's/^v//')

# All environments for assets generation
ALL_ENVS	= esp32dev esp_wroom_02 esp32dev_oled esp_wroom_02_oled

# PlatformIO installation check and activation
PYTHON_VENV	= ~/.platformio/penv
PIO_ACTIVATE	= . $(PYTHON_VENV)/bin/activate &&

# Default target - show help
.DEFAULT_GOAL := help

.PHONY: help all build upload monitor clean install deps lint format check check-pio detect erase _run-pio assets assets-esp32 assets-esp8266 assets-clean assets-manifest

help:	## Show this help
	@echo "YUMA Stratum Proxy - Available targets (BOARD=$(BOARD)):"
	@echo "  - Supported BOARD values: $(SUPPORTED_BOARDS)"
	@echo "  - Override BOARD on the command line, e.g. 'make BOARD=esp32 build'"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' Makefile | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-12s %s\n", $$1, $$2}'

check-pio:	## Check if PlatformIO is installed and available
	@echo "Checking PlatformIO installation..."
	@if command -v $(PLATFORM) >/dev/null 2>&1; then \
		echo "✓ PlatformIO found in PATH"; \
		$(PLATFORM) --version; \
	elif [ -f $(PYTHON_VENV)/bin/activate ] && [ -f $(PYTHON_VENV)/bin/$(PLATFORM) ]; then \
		echo "✓ PlatformIO found in virtual environment"; \
		$(PIO_ACTIVATE) $(PLATFORM) --version; \
	elif [ -f ~/.local/bin/$(PLATFORM) ]; then \
		echo "✓ PlatformIO found in ~/.local/bin"; \
		~/.local/bin/$(PLATFORM) --version; \
	else \
		echo "✗ PlatformIO not found. Install with:"; \
		echo "  curl -fsSL https://raw.githubusercontent.com/platformio/platformio-installer-script/master/get-platformio.py -o get-platformio.py"; \
		echo "  python3 get-platformio.py"; \
		exit 1; \
	fi

all: check-pio build	## Build the project

build: check-pio	## Build the project
	@$(MAKE) --no-print-directory _run-pio ARGS="run --environment $(BUILD_ENV)"

upload: check-pio	## Upload firmware to target board
	@python3 scripts/pio_auto_upload.py $(UPLOAD_ENV_ARGS) $(if $(UPLOAD_PORT),--port $(UPLOAD_PORT),)

monitor: check-pio ## Start serial monitor
	@$(MAKE) --no-print-directory _run-pio ARGS="device monitor --port $(MONITOR_PORT) --baud $(MONITOR_SPEED)"

flash: build upload	## Build and upload firmware

deps: check-pio	## Install project dependencies
	@$(MAKE) --no-print-directory _run-pio ARGS="pkg install"

install: deps	## Install dependencies (alias for deps)

clean: check-pio	## Clean build artifacts
	@$(MAKE) --no-print-directory _run-pio ARGS="run --target clean"
	@rm -rf .pio/build .pio/libdeps 2>/dev/null || true

check: check-pio	## Check project configuration
	@$(MAKE) --no-print-directory _run-pio ARGS="check --environment $(BUILD_ENV)"

# Internal target to run PlatformIO with proper activation
_run-pio:
	@if command -v $(PLATFORM) >/dev/null 2>&1; then \
		$(PLATFORM) $(ARGS); \
	elif [ -f $(PYTHON_VENV)/bin/activate ] && [ -f $(PYTHON_VENV)/bin/$(PLATFORM) ]; then \
		$(PIO_ACTIVATE) $(PLATFORM) $(ARGS); \
	elif [ -f ~/.local/bin/$(PLATFORM) ]; then \
		~/.local/bin/$(PLATFORM) $(ARGS); \
	else \
		echo "✗ PlatformIO not available. Run 'make check-pio' for installation instructions."; \
		exit 1; \
	fi

lint:	## Run code linting
	@echo "No linting configured yet"

format:	## Format source code
	@echo "No formatting configured yet"

detect:	## Auto-detect ESP boards and update configuration
	@echo "Detecting ESP boards (ESP32/ESP8266)..."
	@./detect_board.sh

erase: check-pio	## Erase flash memory completely
	@$(MAKE) --no-print-directory _run-pio ARGS="run --target erase --environment $(BUILD_ENV)"

info:	## Show project information
	@echo "Project: YUMA Stratum Proxy"
	@echo "Version: $(VERSION)"
	@echo "Board: $(BOARD)"
	@echo "Environment: $(BUILD_ENV)"
	@echo "Source: $(SRC_DIR)"
	@echo "Build: $(BUILD_DIR)"
	@echo "Assets: $(ASSETS_DIR)"

# Assets generation targets
assets: check-pio assets-clean	## Generate all firmware binaries for web flasher
	@echo "🏗️  Building all firmware variants for version $(VERSION)..."
	@$(MAKE) --no-print-directory assets-esp32
	@$(MAKE) --no-print-directory assets-esp8266
	@$(MAKE) --no-print-directory assets-manifest
	@echo "✅ All firmware binaries generated in $(ASSETS_DIR)/"

assets-esp32: check-pio	## Generate ESP32 firmware binaries
	@echo "🔧 Building ESP32 firmware variants..."
	@$(MAKE) --no-print-directory _build-and-copy ENV=esp32dev CHIP=esp32 VARIANT=standard
	@$(MAKE) --no-print-directory _build-and-copy ENV=esp32dev_oled CHIP=esp32 VARIANT=oled

assets-esp8266: check-pio	## Generate ESP8266 firmware binaries
	@echo "🔧 Building ESP8266 firmware variants..."
	@$(MAKE) --no-print-directory _build-and-copy ENV=esp_wroom_02 CHIP=esp8266 VARIANT=standard
	@$(MAKE) --no-print-directory _build-and-copy ENV=esp_wroom_02_oled CHIP=esp8266 VARIANT=oled

assets-clean:	## Clean assets directory
	@echo "🧹 Cleaning assets directory..."
	@rm -rf $(ASSETS_DIR)/firmware/*
	@sleep 0.1
	@mkdir -p $(FIRMWARE_DIR)/esp32/{standard,oled} $(FIRMWARE_DIR)/esp8266/{standard,oled}

assets-manifest: ## Generate manifest.json for web flasher
	@echo "📋 Generating manifest.json..."
	@echo '{' > $(ASSETS_DIR)/manifest.json
	@echo '  "name": "YUMA Stratum Proxy",' >> $(ASSETS_DIR)/manifest.json
	@echo '  "version": "$(VERSION_CLEAN)",' >> $(ASSETS_DIR)/manifest.json
	@echo '  "builds": [' >> $(ASSETS_DIR)/manifest.json
	@for chip in esp32 esp8266; do \
		for variant in standard oled; do \
			if [ -f "$(FIRMWARE_DIR)/$$chip/$$variant/yuma-$$chip-$$variant-$(VERSION_CLEAN).bin" ]; then \
				echo "    {" >> $(ASSETS_DIR)/manifest.json; \
				echo "      \"chipFamily\": \"$$chip\"," >> $(ASSETS_DIR)/manifest.json; \
				echo "      \"variant\": \"$$variant\"," >> $(ASSETS_DIR)/manifest.json; \
				echo "      \"parts\": [" >> $(ASSETS_DIR)/manifest.json; \
				echo "        {" >> $(ASSETS_DIR)/manifest.json; \
				echo "          \"path\": \"firmware/$$chip/$$variant/yuma-$$chip-$$variant-$(VERSION_CLEAN).bin\"," >> $(ASSETS_DIR)/manifest.json; \
				echo "          \"offset\": 65536" >> $(ASSETS_DIR)/manifest.json; \
				echo "        }" >> $(ASSETS_DIR)/manifest.json; \
				if [ "$$chip" = "esp32" ] && [ -f "$(FIRMWARE_DIR)/$$chip/$$variant/bootloader.bin" ]; then \
					echo "        ," >> $(ASSETS_DIR)/manifest.json; \
					echo "        {" >> $(ASSETS_DIR)/manifest.json; \
					echo "          \"path\": \"firmware/$$chip/$$variant/bootloader.bin\"," >> $(ASSETS_DIR)/manifest.json; \
					echo "          \"offset\": 4096" >> $(ASSETS_DIR)/manifest.json; \
					echo "        }" >> $(ASSETS_DIR)/manifest.json; \
				fi; \
				echo "      ]" >> $(ASSETS_DIR)/manifest.json; \
				if [ "$$chip" != "esp8266" ] || [ "$$variant" != "oled" ]; then \
					echo "    }," >> $(ASSETS_DIR)/manifest.json; \
				else \
					echo "    }" >> $(ASSETS_DIR)/manifest.json; \
				fi; \
			fi; \
		done; \
	done
	@echo '  ]' >> $(ASSETS_DIR)/manifest.json
	@echo '}' >> $(ASSETS_DIR)/manifest.json
	@echo "✅ Generated $(ASSETS_DIR)/manifest.json"

# Internal target to build and copy firmware
_build-and-copy:
	@echo "  📦 Building $(ENV) ($(CHIP) $(VARIANT))..."
	@$(MAKE) --no-print-directory _run-pio ARGS="run --environment $(ENV)" >/dev/null 2>&1
	@TARGET_DIR="$(FIRMWARE_DIR)/$(CHIP)/$(VARIANT)"; \
	BUILD_PATH=".pio/build/$(ENV)"; \
	mkdir -p "$$TARGET_DIR"; \
	if [ -f "$$BUILD_PATH/firmware.bin" ]; then \
		cp "$$BUILD_PATH/firmware.bin" "$$TARGET_DIR/yuma-$(CHIP)-$(VARIANT)-$(VERSION_CLEAN).bin" && \
		echo "    ✅ $$TARGET_DIR/yuma-$(CHIP)-$(VARIANT)-$(VERSION_CLEAN).bin"; \
	fi; \
	if [ "$(CHIP)" = "esp32" ] && [ -f "$$BUILD_PATH/bootloader.bin" ]; then \
		cp "$$BUILD_PATH/bootloader.bin" "$$TARGET_DIR/" && \
		echo "    ✅ $$TARGET_DIR/bootloader.bin"; \
	fi
