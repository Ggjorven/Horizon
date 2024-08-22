## Install VulkanSDK using "install-vulkansdk.sh"

## Configure enviroment variables
1. Navigate to your /Users/username/ folder.

2. If there's already a file called .zshrc open it. If not create one.

3. Add this to the file: (replace "/VulkanSDK/XXXX/macOS" with your Vulkan install location.)

    ```sh
    export VULKAN_SDK=/VulkanSDK/XXXX/macOS
    export PATH=$VULKAN_SDK/bin:$PATH
    export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH
    export VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json
    export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d
    ```

## FAQ

1. I can't see the .zshrc file.
    Make sure you can see hidden files (Command + Shift + .)
