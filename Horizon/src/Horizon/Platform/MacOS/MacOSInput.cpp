#include "hzpch.h"

#if defined(HZ_PLATFORM_MACOS)
#include "Horizon/Core/Input/Input.hpp"

#include <GLFW/glfw3.h>

#include "Horizon/Core/Window.hpp"

namespace Hz
{

    bool Input::IsKeyPressed(Key keycode)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        int state = glfwGetKey(window, (int)keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMousePressed(MouseButton button)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        int state = glfwGetMouseButton(window, (int)button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetCursorPosition()
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        return { xPos, yPos };
    }

    void Input::SetCursorPosition(glm::vec2 position)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        glfwSetCursorPos(window, position.x, position.y);
    }

    void Input::SetCursorMode(CursorMode mode)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        glfwSetInputMode(window, GLFW_CURSOR, (int)mode);
    }

}
#endif
