#include "Scene.hpp"

namespace Sandbox
{

    Scene::Scene()
    {
        /*
        // Static scene
		TransformComponent transform = {};
		transform.Position = { 0.0f, 0.0f, 0.0f };
		transform.Size = { 1.0f, 1.0f, 1.0f };
		transform.Rotation = { -90.0f, 0.0, 270.0f };

		MeshComponent mesh = {};
		mesh.MeshObject = Mesh::Create("assets/objects/viking_room.obj");
		mesh.Albedo = Image2D::Create({"assets/objects/viking_room.png"});

		PointLightComponent light = {};
		light.Colour = { 0.0f, 1.0f, 1.0f };
		light.Intensity = 1.0f;
		light.Radius = 1.5f;

		// Viking mesh
		entt::entity viking = m_Registry.create();
		m_Registry.emplace<TransformComponent>(viking, transform);
		m_Registry.emplace<MeshComponent>(viking, mesh);

		// Light
		entt::entity pointLight = m_Registry.create();

		transform.Position = { 0.5f, 0.5f, 0.5f };

		m_Registry.emplace<TransformComponent>(pointLight, transform);
		m_Registry.emplace<PointLightComponent>(pointLight, light);

		// Vk 2
		entt::entity vk2 = m_Registry.create();

		transform.Position = { -1.3f, -0.4f, 0.0f };
		transform.Rotation = { 270.0f, 0.0f, 180.0f };

		light.Colour = { 0.76f, 0.15f, 0.15f };
		light.Intensity = 1.4f;
		light.Radius = 0.9f;

		m_Registry.emplace<TransformComponent>(vk2, transform);
		m_Registry.emplace<MeshComponent>(vk2, mesh);
		m_Registry.emplace<PointLightComponent>(vk2, light);
        */
    }

    Scene::~Scene()
    {
    }

}
