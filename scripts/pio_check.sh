#!/bin/bash

# PlatformIO Installation Check and Execution Script
# Checks for PlatformIO installation and executes commands with proper activation

PLATFORM="platformio"
PYTHON_VENV="$HOME/.platformio/penv"
PIO_ACTIVATE=". $PYTHON_VENV/bin/activate &&"

# Function to check PlatformIO installation
check_pio() {
    echo "Checking PlatformIO installation..."

    if command -v $PLATFORM >/dev/null 2>&1; then
        echo "✓ PlatformIO found in PATH"
        $PLATFORM --version
        return 0
    elif [ -f "$PYTHON_VENV/bin/activate" ] && [ -f "$PYTHON_VENV/bin/$PLATFORM" ]; then
        echo "✓ PlatformIO found in virtual environment"
        eval "$PIO_ACTIVATE $PLATFORM --version"
        return 0
    elif [ -f "$HOME/.local/bin/$PLATFORM" ]; then
        echo "✓ PlatformIO found in ~/.local/bin"
        "$HOME/.local/bin/$PLATFORM" --version
        return 0
    else
        echo "✗ PlatformIO not found. Install with:"
        echo "  curl -fsSL https://raw.githubusercontent.com/platformio/platformio-installer-script/master/get-platformio.py -o get-platformio.py"
        echo "  python3 get-platformio.py"
        return 1
    fi
}

# Function to run PlatformIO command with proper activation
run_pio() {
    local args="$*"

    if command -v $PLATFORM >/dev/null 2>&1; then
        $PLATFORM $args
    elif [ -f "$PYTHON_VENV/bin/activate" ] && [ -f "$PYTHON_VENV/bin/$PLATFORM" ]; then
        eval "$PIO_ACTIVATE $PLATFORM $args"
    elif [ -f "$HOME/.local/bin/$PLATFORM" ]; then
        "$HOME/.local/bin/$PLATFORM" $args
    else
        echo "✗ PlatformIO not available. Run 'make check-pio' for installation instructions."
        return 1
    fi
}

# Main execution
case "$1" in
    "check")
        check_pio
        ;;
    "run")
        shift  # Remove 'run' from arguments
        run_pio "$@"
        ;;
    *)
        echo "Usage: $0 {check|run} [pio_arguments...]"
        echo "  check               - Check PlatformIO installation"
        echo "  run [args...]       - Run PlatformIO command with args"
        exit 1
        ;;
esac