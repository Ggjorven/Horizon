# Horizon

**Horizon** is a rendering library written in C++20 using the Vulkan API. The project was written to have near zero impact on Vulkan initialization and use. The project is very simple, but written to be easily expandable.

## Features

- API dependant wrapper classes around basic functionality (VertexBuffers, Renderpasses, etc.)
- Easy to use utility classes.

## Getting Started

### Prerequisites

- C++20 compatible compiler

### Building

Build instructions for this project can be found in the [BUILDING.md](BUILDING.md) file. Supported platforms are Windows (vs2022 & make), Linux (make) & MacOS (XCode).

## License
This project is licensed under the Apache 2.0 License. See [LICENSE](LICENSE.txt) for details.

## Contributing
Contributions are welcome! Please fork the repository and create a pull request with your changes.

## Third-Party Libraries
  - `Pulse` [https://github.com/Ggjorven/Pulse](https://github.com/Ggjorven/Pulse.git)
  - `spdlog` [https://github.com/gabime/spdlog/](https://github.com/gabime/spdlog.git)
  - `stb` [https://github.com/nothings/stb.git](https://github.com/nothings/stb.git)
  - `assimp` [https://github.com/assimp/assimp.git](https://github.com/assimp/assimp.git)
  - `glm` [https://github.com/icaven/glm.git](https://github.com/icaven/glm.git)
  - `vma` [https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
  - `tracy` [https://github.com/wolfpld/tracy.git](https://github.com/wolfpld/tracy.git)
