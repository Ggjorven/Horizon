#pragma once

#include "Horizon/Renderer/RendererSpecification.hpp"

namespace Hz
{

    class VulkanContext;

    class Renderer;

    class GraphicsContext
    {
    public:
        using ContextType = VulkanContext;
    public:
        static void Init(void* window);
        static void Destroy();

        static ContextType* Raw() { return s_Instance;}

    private:
        static ContextType* s_Instance;

        friend class Renderer;
    };


    // A helper macro to put on top of functions to enforce the 'api' to be selected to build the code underneath
    // the ... is for any argument that might be needed to return the function, by default it returns nothing.
    #define ENFORCE_API(api, ...) if (!std::is_same_v<GraphicsContext::ContextType, api##Context>) return __VA_ARGS__

}
