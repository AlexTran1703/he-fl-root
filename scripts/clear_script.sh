#!/bin/bash
# Help message

# Default build directory
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
BUILD_DIR="${SCRIPT_DIR}/../build"
KEY_DIR="${SCRIPT_DIR}/../keys"
LOG_DIR="${SCRIPT_DIR}/../log"

# Function to clear the build directory
clear_all() {
    echo "Removing All..."
    clear_build
    clear_log
    clear_keys
}

clear_build() {
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing $BUILD_DIR..."
        (rm -r ${BUILD_DIR}/* ${BUILD_DIR}.* 2>/dev/null)
        echo "Build directory cleared!"
    else
        echo "No build directory found."
    fi
}
clear_log() {
    if [ -d "$LOG_DIR" ]; then
        echo "Removing $LOG_DIR..."
        (rm -r ${LOG_DIR}/* 2>/dev/null)
        echo "Log directory cleared!"
    else
        echo "No log directory found."
    fi
}
clear_keys() {
    if [ -d "$KEY_DIR" ]; then
        echo "Removing $KEY_DIR..."
        (rm -r ${KEY_DIR}/* 2>/dev/null)
        echo "Keys directory cleared!"
    else
        echo "No Keys directory found."
    fi
}

show_help() {
    echo "Usage: clean.sh [options]"
    echo ""
    echo "Options:"
    echo "  build     Remove the build directory"
    echo "  keys     Remove the keys directory"
    echo "  all     Remove the all directory"
    echo "  --help            Show this help message"
}



# Main script logic
if [ "$1" == "build" ]; then
    clear_build
elif [ "$1" == "keys" ]; then
    clear_keys
elif [ "$1" == "all" ]; then
    clear_all
elif [ "$1" == "help" ]; then
    show_help
else
    clear_log
fi
