#include "hzpch.h"
#include "GraphicsContext.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Vulkan/VulkanContext.hpp"

namespace Hz
{

    GraphicsContext::ContextType* GraphicsContext::s_Instance = nullptr;

    void GraphicsContext::Init(void* window)
    {
        s_Instance = new ContextType(window);
        s_Instance->Init();
    }

    void GraphicsContext::Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }

}
