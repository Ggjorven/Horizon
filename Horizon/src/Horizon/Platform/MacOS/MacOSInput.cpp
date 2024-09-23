#include "hzpch.h"

#if defined(HZ_PLATFORM_MACOS)
#include "Horizon/Core/Input/Input.hpp"

#include <GLFW/glfw3.h>

#include "Horizon/Core/Window.hpp"

namespace Hz
{

    ///////////////////////////////////////////////////////////
    // Keyboard & Mouse
    ///////////////////////////////////////////////////////////
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

    std::string Input::GetClipboard()
    {
        const char* cStr = glfwGetClipboardString(NULL);
        if (cStr)
            return std::string(cStr);

        return {};
    }

    void Input::SetClipboard(const std::string& input)
    {
        glfwSetClipboardString(NULL, input.c_str());
    }

    ///////////////////////////////////////////////////////////
    // Drag & Drop
    ///////////////////////////////////////////////////////////
    static DragDropCallbackFn s_DragDropCallback = nullptr;

    static void GLFWDragDropCallback(GLFWwindow* window, int count, const char** paths)
    {
        if (s_DragDropCallback)
        {
            std::vector<std::filesystem::path> paths;
            paths.reserve((size_t)count);

            for (int i = 0; i < count; i++)
                paths.emplace_back(std::filesystem::path(paths[i]));

            s_DragDropCallback(paths);
        }
    }

    void Input::SetDropCallback(DragDropCallbackFn&& callback)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());

        glfwSetDropCallback(window, &GLFWDragDropCallback);
    }

    ///////////////////////////////////////////////////////////
    // Gamepad & JoyStick
    ///////////////////////////////////////////////////////////
    std::string Input::GetJoyStickName(JoyStick joyStick)
    {
        return std::string(glfwGetJoystickName(static_cast<int>(joyStick)));
    }

    std::string Input::GetGamepadName(JoyStick joyStick)
    {
        return std::string(glfwGetGamepadName(static_cast<int>(joyStick)));
    }

    bool Input::JoyStickPresent(JoyStick joyStick)
    {
        return static_cast<bool>(glfwJoystickPresent(static_cast<int>(joyStick)));
    }

    bool Input::JoyStickIsGamepad(JoyStick joyStick)
    {
        return static_cast<bool>(glfwJoystickIsGamepad(static_cast<int>(joyStick)));
    }

    std::vector<float> Input::GetJoyStickAxes(JoyStick joyStick)
    {
        int count;
        const float* axes = glfwGetJoystickAxes(static_cast<int>(joyStick), &count);

        return std::vector<float>(axes, axes + count);
    }

    std::vector<JoyStickState> Input::GetJoyStickButtons(JoyStick joyStick)
    {
        int count;
        const unsigned char* buttons = glfwGetJoystickButtons(static_cast<int>(joyStick), &count);

        return std::vector<JoyStickState>((JoyStickState*)buttons, (JoyStickState*)buttons + count);
    }

    std::vector<JoyStickHatState> Input::GetJoyStickHats(JoyStick joyStick)
    {
        int count;
        const unsigned char* hats = glfwGetJoystickHats(static_cast<int>(joyStick), &count);

        return std::vector<JoyStickHatState>((JoyStickHatState*)hats, (JoyStickHatState*)hats + count);
    }

    GamepadState Input::GetGamepadState(JoyStick joyStick)
    {
        GLFWgamepadstate glfwState;
        if (!glfwGetGamepadState(static_cast<int>(joyStick), &glfwState))
            return {};

        GamepadState gamepadState = {};

        std::memcpy(gamepadState.Buttons.data(), (JoyStickState*)glfwState.buttons, sizeof(glfwState.buttons));
        std::memcpy(gamepadState.Axes.data(), (JoyStickState*)glfwState.axes, sizeof(glfwState.axes));

        return gamepadState;
    }

}
#endif
