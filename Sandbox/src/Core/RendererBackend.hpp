#pragma once

#include <Horizon/Core/Memory.hpp>
#include <Horizon/Core/Events.hpp>

namespace Sandbox
{

    class RendererBackend : public Hz::RefCounted
    {
    public:
        RendererBackend() = default;
        ~RendererBackend() = default;

        virtual void OnUpdate(float deltaTime) = 0;
        virtual void OnRender() = 0;
        virtual void OnEvent(Hz::Event& e) = 0;
    };

}
