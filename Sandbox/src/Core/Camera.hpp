#pragma once

#include <Horizon/Core/Events.hpp>

#include <glm/glm.hpp>

using namespace Hz;

class Camera : public RefCounted
{
public:
    Camera();
    ~Camera();

    void OnUpdate(float deltaTime);
    void OnEvent(Event& e);

    // Setters & getters
    void SetPosition(const glm::vec3& position);
    inline const glm::vec3& GetPosition() const { return m_Position; }
    
    void SetZoom(float zoom);
    inline float GetZoom() const { return m_Zoom; }
    
    inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

private:
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

private:
    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

    glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    float m_Zoom = 1.0f;
    float m_Rotation = 0.0f;  // Optional: for rotating the camera
};