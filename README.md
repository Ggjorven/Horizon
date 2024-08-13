# Horizon

**Horizon** is a rendering library written in C++20 using the Vulkan API. The project was written to have near zero impact on Vulkan initialization and use. The project is very simple, but written to be easily expandable.

## Features

- API dependant wrapper classes around basic functionality (VertexBuffers, Renderpasses, etc.)
- Easy to use utility classes.

## Getting Started

### Prerequisites

- C++20 compatible compiler

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

### Linux

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

4. (Optional) If you haven't already installed the Vulkan SDK you can install it like this:
    ```sh
    chmod +x install-vulkansdk.sh
    ./install-vulkansdk.sh
    ```

5. Generate make files:
    ```sh
    chmod +x gen-make.sh
    ./gen-make.sh
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
This script works on these architectures (**Ubuntu/Debian**, **Fedora**, **CentOS/RHEL**, **Arch**).

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

## License
This project is licensed under the Apache 2.0 License. See [LICENSE](LICENSE.txt) for details.

## Contributing
Contributions are welcome! Please fork the repository and create a pull request with your changes.

## Third-Party Libraries
  - `Pulse` [https://github.com/Ggjorven/Pulse](https://github.com/Ggjorven/Pulse.git)
  - `spdlog` [https://github.com/gabime/spdlog/](https://github.com/gabime/spdlog.git)
  - `stb` [https://github.com/nothings/stb.git](https://github.com/nothings/stb.git)
  - `glm` [https://github.com/icaven/glm.git](https://github.com/icaven/glm.git)
  - `vma` [https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
  - `premake-vscode` [https://github.com/peter1745/premake-vscode/tree/main](https://github.com/peter1745/premake-vscode.git)
  - `tracy` [https://github.com/wolfpld/tracy.git](https://github.com/wolfpld/tracy.git)
