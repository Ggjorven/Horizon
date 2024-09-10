#include "hzpch.h"

#if defined(HZ_PLATFORM_ANDROID)
#include "Horizon/Core/Window.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Utils/Profiler.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

// TODO: ...

namespace Hz
{

    static WeakRef<Window> s_Window = nullptr;



	Window::Window(const WindowSpecification windowSpecs, const RendererSpecification rendererSpecs)
		: m_Specification(windowSpecs)
	{
		HZ_ASSERT(m_Specification.EventCallback, "No event callback was passed in.");

		// TODO: ...
		
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
		// TODO: ...
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
		// TODO: ...

		GraphicsContext::Destroy();
	}

	void Window::SetVSync(bool vsync)
	{
		Renderer::Recreate(m_Specification.Width, m_Specification.Height, vsync);
	}

	void Window::SetTitle(const std::string& title)
	{
		m_Specification.Title = title;
		// TODO: ...
	}

	std::pair<float, float> Window::GetPosition() const
	{
		HZ_ASSERT(false, "On Android the screen doesn't have a position, so should not be queried.");
		return std::pair<float, float>((float)0.0f, (float)0.0f);
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
