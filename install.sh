#!/bin/bash

# Install script for babygit
set -e  # Exit on error

# Variables
PROJECT_DIR="$(dirname "$(realpath "$0")")"
BIN_NAME="babygit"
INSTALL_DIR="/usr/local/bin"
BUILD_DIR="$PROJECT_DIR/bin"

echo "=== Installing $BIN_NAME ==="

# Step 1: Compile the project
echo "Compiling $BIN_NAME..."
cd "$PROJECT_DIR"
make clean
if ! make; then
    echo "Error: Compilation failed" >&2
    exit 1
fi

# Step 2: Verify the binary was built
if [[ ! -f "$BUILD_DIR/$BIN_NAME" ]]; then
    echo "Error: Binary not found at $BUILD_DIR/$BIN_NAME" >&2
    exit 1
fi

# Step 3: Install system-wide
echo "Installing to $INSTALL_DIR..."
sudo cp "$BUILD_DIR/$BIN_NAME" "$INSTALL_DIR/"
sudo chmod +x "$INSTALL_DIR/$BIN_NAME"

# Step 4: Verify installation
if ! command -v "$BIN_NAME" >/dev/null; then
    echo "Error: Installation failed - $BIN_NAME not in PATH" >&2
    exit 1
fi

echo "Success! $BIN_NAME is now installed."
echo "Run with: $BIN_NAME --help"
