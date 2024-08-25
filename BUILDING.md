## Installation

### Windows

1. Clone the repository:
    ```sh
    git clone --recursive https://github.com/ggjorven/Horizon.git
    cd Horizon
    ```

2. Navigate to the scripts folder:
    ```sh
    cd scripts/windows
    ```

3. (Optional) If you haven't already installed the Vulkan SDK you can install it like this:
    ```sh
    ./install-vulkansdk.bat
    ```

4. Choose what you want it build to:
    - Visual Studio 17 2022:
        ```sh
        ./gen-vs2022.bat
        ```
    - MinGW make files:
        ```sh
        ./gen-make.bat
        ```

### Linux (X11 & Wayland)

1. Clone the repository:
    ```sh
    git clone --recursive https://github.com/ggjorven/Horizon.git
    cd Horizon
    ```

2. Navigate to the scripts folder:
    ```sh
    cd scripts/linux
    ```

3. (Optional) If you haven't already installed the premake5 build system you can install it like this:
    ```sh
    chmod +x install-premake5.sh
    ./install-premake5.sh
    ```

4. (Optional) If you haven't already installed the Vulkan SDK you can find installation instructions in [VulkanHelp.md](scripts/macos/VulkanHelp.md).

5. Generate make files:
    ```sh
    chmod +x gen-make.sh
    ./gen-make.sh
    ```

### MacOS

1. Clone the repository:
    ```sh
    git clone --recursive https://github.com/ggjorven/Horizon.git
    cd Horizon
    ```

2. Navigate to the scripts folder:
    ```sh
    cd scripts/macos
    ```

3. (Optional) If you haven't already installed the premake5 build system you can install it like this:
    ```sh
    chmod +x install-premake5.sh
    ./install-premake5.sh
    ```

4. (Optional) If you haven't already installed the Vulkan SDK you can find installation instructions in [VulkanHelp.md](scripts/macos/VulkanHelp.md).

5. Generate XCode project files:
    ```sh
    chmod +x gen-xcode.sh
    ./gen-xcode.sh
    ```

## Building

### Windows
- Visual Studio 17 2022:
    1. Navigate to the root of the directory
    2. Open the Pulse.sln file
    3. Start building in your desired configuration
    4. Build files can be in the bin/%Config%-windows/ folder.
    5. (Optional) Open a terminal and run the Sandbox project:

        ```sh
        ./Sandbox.exe
        ```

- MinGW Make:
    1. Navigate to the root of the directory
    2. Open a terminal.
    3. Call make with desired configuration (debug, release or dist):

        ```sh
        make config=release
        ```

    5. Build files can be in the bin/%Config%-linux/ folder.
    6. (Optional) Open a terminal and run the Sandbox project:
        ```sh
        ./Sandbox.exe
        ```

### Linux
Before you start make sure you have GLFW dependencies installed:
- libx11, libxcursor, libxrandr, libxinerama, libxi

If you don't have these installed we have created a script to help you out.
This script is tested on **Ubuntu/Debian** & **Linux Mint**. If you use another version of linux, you will need to use your package manger to install the packages listed above on your own.

This scripts can be found in the 'scripts/linux/' folder, then run these commands:
```sh
chmod +x install-glfw-dependencies.sh
./install-glfw-dependencies.sh
```

#### Actual build:

1. Navigate to the root of the directory
2. Open a terminal
3. Call make with desired configuration (debug, release or dist):

    ```sh
    make config=release
    ```

5. Build files can be in the bin/%Config%-linux/ folder.
6. (Optional) Open a terminal and run the Sandbox project:

    ```sh
    chmod +x Sandbox
    ./Sandbox
    ```

### MacOS

1. Navigate to the root of the directory
2. Open the Horizon.xcworkspace file
3. Start building in your desired configuration
4. Build files can be in the bin/%Config%-macosx/ folder.
5. (Optional) Open a terminal and run the Sandbox project:

    ```sh
    chmod +x Sandbox
    ./Sandbox
    ```