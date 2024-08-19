#pragma once

#include <Horizon/Core/Events.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

using namespace Hz;

class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	void OnUpdate(float deltaTime);

	inline glm::vec3& GetPosition() { return m_Position; }

	inline glm::mat4& GetViewMatrix() { return m_View; }
	inline glm::mat4& GetProjectionMatrix() { return m_Projection; }

private:
	void UpdateMatrices();

private:
	// All
	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
	float m_FOV = 45.0f;

	float m_Near = 0.1f;
	float m_Far = 1000.0f;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;

	glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
	glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
	glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };

	// Flycam
	float m_MovementSpeed = 5.0f;
	float m_MouseSensitivity = 0.1f;

	bool m_FirstUpdate = true;

	// Matrices
	glm::mat4 m_View = {};
	glm::mat4 m_Projection = {};
	// glm::vec2 m_DepthUnpackConsts = {};
};