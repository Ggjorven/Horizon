#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Sandbox
{

    struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

	private:
		glm::vec3 RotationEuler = { 0.0f, 0.0f, 0.0f };
		glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };

	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		~TransformComponent() = default;

		void SetTransform(const glm::mat4& transform);
		glm::mat4 GetTransform() const;

		void SetRotationEuler(const glm::vec3& euler);
		glm::vec3 GetRotationEuler() const;

		void SetRotation(const glm::quat& quat);
		glm::quat GetRotation() const;
	};

    /*
    struct MeshComponent
    {
    public:
        Ref<Mesh> MeshObject = nullptr;
        Ref<Image2D> Albedo = nullptr;

    public:
        MeshComponent() = default;
        MeshComponent(Ref<Mesh> mesh, Ref<Image2D> albedo);
        MeshComponent(const MeshComponent& other) = default;
    };
    */

    struct PointLightComponent
    {
    public:
        glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };
        float Radius = 2.0f;
        float Intensity = 1.0f;

    public:
        PointLightComponent() = default;
        PointLightComponent(const glm::vec3& colour, float radius, float intensity);
        PointLightComponent(const PointLightComponent& other) = default;
    };

}
