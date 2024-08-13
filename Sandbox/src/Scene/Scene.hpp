#pragma once

#include <Pulse/ECS/EnTT.hpp>

namespace Sandbox
{

    class Scene
    {
    public:
        Scene();
        ~Scene();

        inline entt::registry& GetRegistry() { return m_Registry; }

    private:
        entt::registry m_Registry;
    };

}
