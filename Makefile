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

.PHONY: help all build upload monitor clean install deps lint format check check-pio detect erase _run-pio assets assets-esp32 assets-esp8266 assets-clean manifest

help:	## Show this help
	@echo "YUMA Stratum Proxy - Available targets (BOARD=$(BOARD)):"
	@echo "  - Supported BOARD values: $(SUPPORTED_BOARDS)"
	@echo "  - Override BOARD on the command line, e.g. 'make BOARD=esp32 build'"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' Makefile | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-12s %s\n", $$1, $$2}'

check-pio:	## Check if PlatformIO is installed and available
	@./scripts/pio_check.sh check

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
	@./scripts/pio_check.sh run $(ARGS)

lint:	## Run code linting
	@echo "No linting configured yet"

format:	## Format source code
	@echo "No formatting configured yet"

detect:	## Auto-detect ESP boards and update configuration
	@echo "Detecting ESP boards (ESP32/ESP8266)..."
	@./scripts/detect_board.sh

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
assets: check-pio	## Generate all firmware binaries for web flasher
	@./scripts/generate_assets.sh all

assets-esp32: check-pio	## Generate ESP32 firmware binaries
	@./scripts/generate_assets.sh esp32

assets-esp8266: check-pio	## Generate ESP8266 firmware binaries
	@./scripts/generate_assets.sh esp8266

assets-clean:	## Clean assets directory
	@./scripts/generate_assets.sh clean

manifest: ## Generate manifest.json for web flasher
	@./scripts/generate_manifest.sh generate
