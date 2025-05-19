#!/bin/bash

# Uninstall script for babygit
set -e  # Exit on error

# Variables
BIN_NAME="babygit"
INSTALL_DIR="/usr/local/bin"
LOCAL_INSTALL_DIR="$HOME/.local/bin"

echo "=== Uninstalling $BIN_NAME ==="

# Check which installation exists
if [[ -f "$INSTALL_DIR/$BIN_NAME" ]]; then
    echo "Removing system-wide installation..."
    sudo rm -f "$INSTALL_DIR/$BIN_NAME"
elif [[ -f "$LOCAL_INSTALL_DIR/$BIN_NAME" ]]; then
    echo "Removing user installation..."
    rm -f "$LOCAL_INSTALL_DIR/$BIN_NAME"
else
    echo "Error: $BIN_NAME not found in either:"
    echo "- $INSTALL_DIR/"
    echo "- $LOCAL_INSTALL_DIR/"
    exit 1
fi

# Verify removal
if command -v "$BIN_NAME" >/dev/null; then
    echo "Warning: $BIN_NAME still found in PATH (may exist elsewhere)"
else
    echo "Successfully removed $BIN_NAME"
fi

# Optional: Clean build artifacts
read -p "Remove build artifacts? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Cleaning project..."
    make clean >/dev/null
    rmdir bin 2>/dev/null || true
fi

echo "Uninstall complete."
