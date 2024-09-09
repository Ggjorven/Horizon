#include "hzpch.h"

#if defined(HZ_PLATFORM_MACOS)
#include "Horizon/Core/Window.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Utils/Profiler.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include <GLFW/glfw3.h>

namespace Hz
{

	static bool s_GLFWInitialized = false;
	static WeakRef<Window> s_Window = nullptr;

	static void GLFWErrorCallBack(int errorCode, const char* description)
	{
		HZ_LOG_ERROR("[GLFW]: ({0}), {1}", errorCode, description);
	}



	Window::Window(const WindowSpecification windowSpecs, const RendererSpecification rendererSpecs)
		: m_Specification(windowSpecs)
	{
		HZ_ASSERT(m_Specification.EventCallback, "No event callback was passed in.");

		if (!s_GLFWInitialized)
		{
			int result = glfwInit();
			HZ_ASSERT((result), "[GLFW] glfwInit() failed!");

			s_GLFWInitialized = true;
			glfwSetErrorCallback(GLFWErrorCallBack);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Window = glfwCreateWindow((int)windowSpecs.Width, (int)windowSpecs.Height, windowSpecs.Title.c_str(), nullptr, nullptr);
		HZ_ASSERT(m_Window, "Failed to create window...");

		GraphicsContext::Init(m_Window, windowSpecs.Width, windowSpecs.Height, rendererSpecs.VSync, (uint8_t)rendererSpecs.Buffers);

		GLFWwindow* window = static_cast<GLFWwindow*>(m_Window);
		glfwSetWindowUserPointer(window, (void*)&m_Specification); //So we can access/get to the data in lambda functions

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event = WindowResizeEvent(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				WindowCloseEvent event = WindowCloseEvent();
				data.EventCallback(event);
			});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event = KeyPressedEvent(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event = KeyReleasedEvent(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event = KeyPressedEvent(key, 1);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event = KeyTypedEvent(keycode);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event = MouseButtonPressedEvent(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event = MouseButtonReleasedEvent(button);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event = MouseScrolledEvent((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event = MouseMovedEvent((float)xPos, (float)yPos);
				data.EventCallback(event);
			});

		Renderer::Init(rendererSpecs);
	}

	Window::~Window()
	{
		if (m_Window)
			ForceClose();
	}

	void Window::PollEvents()
	{
		HZ_MARK_FRAME();
		glfwPollEvents();
	}

	void Window::SwapBuffers()
	{
		HZ_PROFILE_SCOPE("SwapBuffers");
		Renderer::Present();
	}

	void Window::Close()
	{
		m_Closed = true;
	}

	void Window::ForceClose()
	{
		m_Closed = true;

		Renderer::Destroy();

		GraphicsContext::Destroy();

		glfwDestroyWindow(static_cast<GLFWwindow*>(m_Window));
		m_Window = nullptr;

		glfwTerminate();
		s_GLFWInitialized = false;
	}

	void Window::SetVSync(bool vsync)
	{
		Renderer::Recreate(m_Specification.Width, m_Specification.Height, vsync);
	}

	void Window::SetTitle(const std::string& title)
	{
		m_Specification.Title = title;
		glfwSetWindowTitle(static_cast<GLFWwindow*>(m_Window), m_Specification.Title.c_str());
	}

	std::pair<float, float> Window::GetPosition() const
	{
		int xPos = 0, yPos = 0;
		glfwGetWindowPos(static_cast<GLFWwindow*>(m_Window), &xPos, &yPos);

		return std::pair<float, float>((float)xPos, (float)yPos);
	}

	Window& Window::Get()
	{
		HZ_ASSERT((s_Window.IsValid()), "Tried to get window which doesn't exist.");
		return *s_Window.GetRef();
	}

	Ref<Window> Window::Create(const WindowSpecification windowSpecs, const RendererSpecification rendererSpecs)
	{
		Ref<Window> window = Ref<Window>::Create(windowSpecs, rendererSpecs);
		s_Window = window;
		return window;
	}

}
#endif