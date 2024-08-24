## Install vulkan drivers

1. Use (Homebrew)[https://brew.sh/] to install vulkan driver: (Or install it using your package manager of choice)
    ```sh
    brew install vulkan-tools
    ```

## Install VulkanSDK

1. Open a terminal in the scripts/macos folder.

2. Run the following commands:
    ```sh
    chmod +x install-vulkansdk.sh
    ./install-vulkansdk.sh
    ```

## Configure enviroment variables
1. Navigate to your /Users/username/ folder.

2. If there's already a file called .zshrc open it. If not create one. (Note: The .zshrc is hidden by default, unhide using: Command + Shift + .)

3. Add this to the file: (replace "/VulkanSDK/XXXX/macOS" with your Vulkan install location.)

    ```sh
    export VULKAN_SDK=/VulkanSDK/XXXX/macOS
    export PATH=$VULKAN_SDK/bin:$PATH
    export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH
    export VK_ICD_FILENAMES=$VULKAN_SDK/share/vulkan/icd.d/MoltenVK_icd.json
    export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
    ```