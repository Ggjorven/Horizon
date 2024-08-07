@echo off
setlocal

:: Set Vulkan SDK version and URLs
set VULKAN_SDK_VERSION=1.3.290.0
set VULKAN_SDK_URL=https://sdk.lunarg.com/sdk/download/%VULKAN_SDK_VERSION%/windows/VulkanSDK-%VULKAN_SDK_VERSION%-Installer.exe
set DOWNLOAD_DIR=%TEMP%\VulkanSDK
set INSTALLER_FILE=%DOWNLOAD_DIR%\VulkanSDK-%VULKAN_SDK_VERSION%-Installer.exe
set INSTALL_DIR=C:\VulkanSDK\%VULKAN_SDK_VERSION%

:: Create download directory
mkdir "%DOWNLOAD_DIR%"

:: Download Vulkan SDK installer using PowerShell
echo Downloading Vulkan SDK from %VULKAN_SDK_URL%...
powershell -Command "Invoke-WebRequest -Uri '%VULKAN_SDK_URL%' -OutFile '%INSTALLER_FILE%'"

:: Check if download was successful
if errorlevel 1 (
    echo Failed to download Vulkan SDK installer.
    exit /b 1
)

:: Run Vulkan SDK installer
echo Running Vulkan SDK installer...
start /wait "" "%INSTALLER_FILE%" /S /D=%INSTALL_DIR%

:: Check if installation was successful
if errorlevel 1 (
    echo Vulkan SDK installation failed.
    exit /b 1
)

:: Cleanup
echo Cleaning up...
rmdir /s /q "%DOWNLOAD_DIR%"

:: Set environment variables
echo Setting environment variables...
setx VULKAN_SDK "%INSTALL_DIR%"
setx PATH "%INSTALL_DIR%\Bin;%PATH%"

echo Vulkan SDK installation completed successfully!

endlocal
pause