#!/bin/bash

# Define variables
VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk-linux-x86_64.tar.xz"
TEMP_DIR="vulkan-sdk-temp"
FINAL_DIR="/usr/local/vulkan-sdk"
TAR_FILE="${TEMP_DIR}.tar.xz"

# Function to handle errors and cleanup
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TAR_FILE" "$TEMP_DIR"
    exit 1
}

# Create necessary directories
mkdir -p "$TEMP_DIR"

# Download the Vulkan SDK tarball
echo "Downloading Vulkan SDK..."
wget -O "$TAR_FILE" "$VULKAN_SDK_URL"
if [ $? -ne 0 ]; then
    echo "Error downloading Vulkan SDK. Check the URL or your network connection."
    cleanup
fi

# Verify the file type
echo "Verifying downloaded file..."
file "$TAR_FILE"
if ! file "$TAR_FILE" | grep -q "XZ compressed data"; then
    echo "The downloaded file is not a valid XZ tarball. Check the download URL or try again."
    cleanup
fi

# Extract the tarball
echo "Extracting Vulkan SDK..."
tar -xJvf "$TAR_FILE" -C "$TEMP_DIR" --no-same-owner -- no-symlinks
if [ $? -ne 0 ]; then
    echo "Error extracting Vulkan SDK. The tarball might be corrupted or the extraction command failed."
    cleanup
fi

# Find the extracted version directory
VERSION_DIR=$(find "$TEMP_DIR" -mindepth 1 -maxdepth 1 -type d)
if [ -z "$VERSION_DIR" ]; then
    echo "Error: Could not find the version directory. Here are the contents of $TEMP_DIR:"
    ls -l "$TEMP_DIR"
    cleanup
fi

# Check if setup-env.sh exists in the version directory
SETUP_ENV_SCRIPT="$VERSION_DIR/setup-env.sh"
if [ ! -f "$SETUP_ENV_SCRIPT" ]; then
    echo "Error: setup-env.sh not found in the version directory. Checking contents of $VERSION_DIR:"
    ls -l "$VERSION_DIR"
    cleanup
fi

# Attempt to create the final directory with sudo
echo "Moving Vulkan SDK to $FINAL_DIR..."
if ! sudo mkdir -p "$FINAL_DIR"; then
    echo "Failed to create $FINAL_DIR. Ensure you have permission to write to /usr/local."
    cleanup
fi

# Move files with sudo
if ! sudo cp -r "$VERSION_DIR/"* "$FINAL_DIR"; then
    echo "Failed to move files to $FINAL_DIR. Ensure you have permission to write to /usr/local."
    cleanup
fi

# Clean up
echo "Cleaning up..."
rm -rf "$TAR_FILE"
rm -rf "$TEMP_DIR"

# Source the environment script from the final location
SETUP_ENV_SCRIPT="$FINAL_DIR/setup-env.sh"
if [ -f "$SETUP_ENV_SCRIPT" ]; then
    echo "Setting up environment variables..."
    # Use a subshell to source the environment script so it affects the current script session
    source "$SETUP_ENV_SCRIPT"

    # Debug output
    echo "Checking VULKAN_SDK after sourcing setup-env.sh..."
    echo "VULKAN_SDK is: $VULKAN_SDK"

    # Add sourcing to .bashrc if it's not already present
    if ! grep -q "source /usr/local/vulkan-sdk/setup-env.sh" ~/.bashrc; then
        echo "source /usr/local/vulkan-sdk/setup-env.sh" >> ~/.bashrc
        echo "Added source command to ~/.bashrc."
    fi
else
    echo "Error: setup-env.sh not found in $FINAL_DIR. Environment variables not set."
    exit 1
fi

echo "Vulkan SDK installation completed successfully!"
echo "The Vulkan SDK has been moved to $FINAL_DIR."
echo "The environment variables have been set. Please restart your terminal or run 'source $FINAL_DIR/setup-env.sh' to finalize the setup."
