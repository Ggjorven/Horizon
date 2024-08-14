#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

namespace Hz
{

    class CommandBuffer : public RefCounted
    {
    public:
        CommandBuffer() = default;
        virtual ~CommandBuffer() = default;

        // The Begin, End & Submit methods are in the Renderer class

        static Ref<CommandBuffer> Create();
    };

}
