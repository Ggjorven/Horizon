#!/bin/bash

# Define the Vulkan SDK version
VULKAN_SDK_VERSION="1.3.290.0"

# Download Vulkan SDK
echo "Downloading Vulkan SDK version $VULKAN_SDK_VERSION..."
curl -LO https://sdk.lunarg.com/sdk/download/$VULKAN_SDK_VERSION/mac/vulkansdk-macos-$VULKAN_SDK_VERSION.dmg

# Mount the Vulkan SDK DMG
echo "Mounting Vulkan SDK DMG..."
MOUNT_POINT=$(hdiutil attach vulkansdk-macos-$VULKAN_SDK_VERSION.dmg | grep Volumes | awk '{print $3}')

if [ -z "$MOUNT_POINT" ]; then
    echo "Failed to mount the DMG file. Exiting."
    exit 1
fi

# Run the Vulkan SDK Installer
echo "Running Vulkan SDK Installer..."
INSTALLER="$MOUNT_POINT/InstallVulkan.app/Contents/MacOS/InstallVulkan"
if [ -f "$INSTALLER" ]; then
    chmod +x "$INSTALLER"
    "$INSTALLER"
else
    echo "Installer not found at $INSTALLER. Exiting."
    hdiutil detach "$MOUNT_POINT"
    exit 1
fi

# Unmount the DMG
echo "Unmounting the DMG..."
hdiutil detach "$MOUNT_POINT"

echo "Deleting .dmg"
rm vulkansdk-macos-$VULKAN_SDK_VERSION.dmg

echo "Vulkan SDK $VULKAN_SDK_VERSION installation complete."
