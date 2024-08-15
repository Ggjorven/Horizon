#include "hzpch.h"
#include "VulkanContext.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

///////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		HZ_LOG_WARN("Validation layer: {0}", pCallbackData->pMessage);
		return VK_FALSE;
	}

	return VK_FALSE;
}

bool ValidationLayersSupported()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Check if all requested layers are actually accessible
    for (const char* layerName : Hz::VulkanContext::s_RequestedValidationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

namespace Hz
{

    const std::vector<const char*> VulkanContext::s_RequestedValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> VulkanContext::s_RequestedDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    void VulkanContext::Init(void* window, uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight)
    {
        s_Data = new Info();
        s_Data->Window = window;

        InitInstance();
        InitDevices(width, height, vsync, framesInFlight);
    }

    void VulkanContext::Destroy()
    {
        Renderer::FreeObjects();

        s_Data->SwapChain.Reset();

        Renderer::FreeObjects();
        VkUtils::Allocator::Destroy();

        s_Data->PhysicalDevice.Reset();
        s_Data->Device.Reset();

        if constexpr (s_Validation)
        {
            if (s_Data->DebugMessenger)
                DestroyDebugUtilsMessengerEXT(s_Data->VulkanInstance, s_Data->DebugMessenger, nullptr);
        }

        vkDestroyInstance(s_Data->VulkanInstance, nullptr);

        // Delete data
        delete s_Data;
        s_Data = nullptr;
    }

    void VulkanContext::InitInstance()
    {
        ///////////////////////////////////////////////////////////
		// Instance Creation
		///////////////////////////////////////////////////////////
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "CherryTree Application";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, Version.first, Version.second, 0);
		appInfo.pEngineName = "CherryTree Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, Version.first, Version.second, 0);
		appInfo.apiVersion = VK_MAKE_API_VERSION(0, Version.first, Version.second, 0);

		#if defined(HZ_PLATFORM_WINDOWS)
			#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
		#elif defined(HZ_PLATFORM_LINUX)
			#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
		#endif // TODO: MacOS

		std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		if constexpr (s_Validation)
		{
            if (!ValidationLayersSupported())
            {
                HZ_LOG_ERROR("Validation layers are not supported!");
            }
            else
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Very little performance hit, can be used in Release.
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
		}

        #if defined(HZ_PLATFORM_MACOS)
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		#if defined(HZ_PLATFORM_MACOS)
			createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		#endif
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();

		if constexpr (s_Validation)
		{
            if (ValidationLayersSupported())
            {
			    createInfo.enabledLayerCount = static_cast<uint32_t>(s_RequestedValidationLayers.size());
			    createInfo.ppEnabledLayerNames = s_RequestedValidationLayers.data();
            }
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Note(Jorben): Setup the debug messenger also for the create instance
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = VulkanDebugCallback;

		if constexpr (s_Validation)
		{
            if (ValidationLayersSupported())
            {
			    createInfo.enabledLayerCount = static_cast<uint32_t>(s_RequestedValidationLayers.size());
			    createInfo.ppEnabledLayerNames = s_RequestedValidationLayers.data();

			    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &s_Data->VulkanInstance));

		///////////////////////////////////////////////////////////
		// Debugger Creation
		///////////////////////////////////////////////////////////
		if constexpr (s_Validation)
		{
            if (ValidationLayersSupported())
            {
			    VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(s_Data->VulkanInstance, &debugCreateInfo, nullptr, &s_Data->DebugMessenger));
            }
        }
    }

    void VulkanContext::InitDevices(uint32_t width, uint32_t height, const bool vsync, const uint8_t framesInFlight)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
		VK_CHECK_RESULT(glfwCreateWindowSurface(s_Data->VulkanInstance, static_cast<GLFWwindow*>(s_Data->Window), nullptr, &surface));

        ///////////////////////////////////////////////////////////
		// Other
		///////////////////////////////////////////////////////////
		s_Data->PhysicalDevice = VulkanPhysicalDevice::Select(surface);
		s_Data->Device = VulkanDevice::Create(surface, s_Data->PhysicalDevice);

		VkUtils::Allocator::Init();

        s_Data->SwapChain = VulkanSwapChain::Create(surface);
        s_Data->SwapChain->Init(width, height, vsync, framesInFlight);
    }

}
