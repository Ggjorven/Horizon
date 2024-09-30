#include "Camera.hpp"

#include <Horizon/Core/Window.hpp>
#include <Horizon/Core/Input/Input.hpp>

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    RecalculateProjectionMatrix();
}

Camera::~Camera()
{
}

void Camera::OnUpdate(float deltaTime)
{
    constexpr const float s_DeltaMovement = 1.0f;

    if (Input::IsKeyPressed(Key::W))
    {
        m_Position.y += s_DeltaMovement * deltaTime;
    }
    if (Input::IsKeyPressed(Key::A))
    {
        m_Position.x -= s_DeltaMovement * deltaTime;
    }
    if (Input::IsKeyPressed(Key::S))
    {
        m_Position.y -= s_DeltaMovement * deltaTime;
    }
    if (Input::IsKeyPressed(Key::D))
    {
        m_Position.x += s_DeltaMovement * deltaTime;
    }

    RecalculateViewMatrix();
}

void Camera::OnEvent(Event& e)
{
    EventHandler handler(e);

    handler.Handle<MouseScrolledEvent>([this](MouseScrolledEvent& e) -> bool
    {
        constexpr const float s_BaseZoomSpeed = 0.25f;

        SetZoom(m_Zoom - (e.GetYOffset() * s_BaseZoomSpeed));
        return false;
    });
    handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
    {
        RecalculateProjectionMatrix();
        return false;
    });
}

void Camera::SetPosition(const glm::vec3& position)
{
    m_Position = position;
    RecalculateViewMatrix();
}

void Camera::SetZoom(float zoom)
{
    constexpr const float s_MinZoom = 0.25f;

    m_Zoom = std::max(s_MinZoom, zoom);
    RecalculateProjectionMatrix();
}

void Camera::RecalculateViewMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

    m_ViewMatrix = glm::inverse(transform);
}

void Camera::RecalculateProjectionMatrix()
{
    auto& window = Window::Get();
    float aspectRatio = (float)window.GetWidth() / window.GetHeight();

    m_ProjectionMatrix = glm::ortho(-(float)aspectRatio * m_Zoom, (float)aspectRatio * m_Zoom, -1.0f * m_Zoom, 1.0f * m_Zoom, -1.0f, 1.0f);
    m_ProjectionMatrix[1][1] *= -1.0f;
}
