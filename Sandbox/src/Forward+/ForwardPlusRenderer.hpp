#pragma once

#include "Core/RendererBackend.hpp"

namespace Sandbox
{

    class ForwardPlusRenderer : public RendererBackend
    {
    public:
        ForwardPlusRenderer();
        ~ForwardPlusRenderer();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnEvent(Hz::Event& e) override;

    private:

    };

}
