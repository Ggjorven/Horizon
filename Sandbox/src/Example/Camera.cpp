#include "Camera.hpp"

#include <Horizon/Core/Logging.hpp>
#include <Horizon/Core/Window.hpp>
#include <Horizon/Core/Input/Input.hpp>

#include <Horizon/Renderer/RendererSpecification.hpp>
#include <Horizon/Renderer/Renderer.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::OnUpdate(float deltaTime)
{
    Window& window = Window::Get();

    if (!window.IsMinimized())
    {
        float velocity = m_MovementSpeed * deltaTime;
        glm::vec3 moveDirection = { 0.0f, 0.0f, 0.0f };

        // Calculate forward/backward and left/right movement.
        if (Input::IsKeyPressed(Key::W))
            moveDirection += m_Front;
        if (Input::IsKeyPressed(Key::S))
            moveDirection -= m_Front;
        if (Input::IsKeyPressed(Key::A))
            moveDirection -= m_Right;
        if (Input::IsKeyPressed(Key::D))
            moveDirection += m_Right;

        // Calculate up/down movement.
        if (Input::IsKeyPressed(Key::Space))
            moveDirection += m_Up;
        if (Input::IsKeyPressed(Key::LeftShift))
            moveDirection -= m_Up;

        if (glm::length(moveDirection) > 0.0f)
            moveDirection = glm::normalize(moveDirection);

        // Update the camera position.
        m_Position += moveDirection * velocity;

        // Mouse movement
        if (Input::IsKeyPressed(Key::LeftAlt))
        {
            static glm::vec2 lastMousePosition = Input::GetCursorPosition();
            if (m_FirstUpdate)
            {
                lastMousePosition = Input::GetCursorPosition();
                m_FirstUpdate = false;
            }

            glm::vec2 mousePosition = Input::GetCursorPosition();
            float xOffset = static_cast<float>(mousePosition.x - lastMousePosition.x);
            float yOffset = static_cast<float>(lastMousePosition.y - mousePosition.y);

            // Reset cursor
            Input::SetCursorPosition({ window.GetWidth() / 2.0f, window.GetHeight() / 2.0f });
            Input::SetCursorMode(CursorMode::Disabled);

            // Update last mouse position
            lastMousePosition = { window.GetWidth() / 2.0f, window.GetHeight() / 2.0f };

            xOffset *= m_MouseSensitivity;
            yOffset *= m_MouseSensitivity;

            // Update yaw and pitch
            m_Yaw += xOffset;
            m_Pitch += yOffset;

            // Cap movement
            if (m_Pitch > 89.0f)
                m_Pitch = 89.0f;
            else if (m_Pitch < -89.0f)
                m_Pitch = -89.0f;

            if (m_Yaw > 360.0f)
                m_Yaw -= 360.0f;
            else if (m_Yaw < -360.0f)
                m_Yaw += 360.0f;
        }
        else
        {
            Input::SetCursorMode(CursorMode::Shown);
            m_FirstUpdate = true;
        }

        UpdateMatrices();
    }
}

void Camera::UpdateMatrices()
{
    Window& window = Window::Get();

    glm::vec3 newFront(1.0f);
    newFront.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    newFront.y = glm::sin(glm::radians(m_Pitch));
    newFront.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(newFront);
    m_Right = glm::normalize(glm::cross(m_Front, m_Up));

    // Update everything
    m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    m_Projection = glm::perspective(glm::radians(m_FOV), (float)window.GetWidth() / (float)window.GetHeight(), m_Near, m_Far);

    if constexpr (RendererSpecification::API == RenderingAPI::Vulkan)
        m_Projection[1][1] *= -1;
}
