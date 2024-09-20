#include "hzpch.h"
#include "Application.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Utils/Profiler.hpp"

#include <Pulse/Time/Timer.hpp>

namespace Hz
{

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& appInfo)
		: m_AppInfo(appInfo)
	{
		s_Instance = this;

		Log::Init();

		m_Extensions.OnInitBegin();

		// This also intializes the renderer
		WindowSpecification windowSpecs = appInfo.WindowSpecs;
		windowSpecs.EventCallback = [this](Event& e) { OnEvent(e); };
		m_Window = Window::Create(windowSpecs, appInfo.RendererSpecs);

		m_Extensions.OnInitEnd();
	}

	Application::~Application()
	{
		m_Extensions.OnDestroyBegin();

		// This also destroys the renderer
		m_Window.Reset();

		m_Extensions.OnDestroyEnd();
	}

	void Application::OnEvent(Event& e)
	{
		EventHandler handler(e);

		handler.Handle<WindowCloseEvent>([this](WindowCloseEvent& e) -> bool { return OnWindowClose(e); });
		handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool { return OnWindowResize(e); });

		m_Extensions.OnEvent(e);
	}

	void Application::Run()
	{
		HZ_ASSERT(m_AppLayer, "Tried to run the application without setting an application layer.");

		Pulse::Time::Timer deltaTimer = {};
		float deltaTime = 0.0f;

		while (m_Running)
		{
			deltaTime = (float)deltaTimer.ElapsedSeconds();

			// Update & Render
			{
				HZ_PROFILE_SCOPE("Renderer::Begin");
				Renderer::BeginFrame();
			}

			{
				HZ_PROFILE_SCOPE("Update");

				m_Extensions.OnUpdateBegin(deltaTime);
				m_AppLayer->OnUpdate(deltaTime);
				m_Extensions.OnUpdateEnd(deltaTime);
			}

			if (!m_Minimized)
			{
				HZ_PROFILE_SCOPE("Render");

				m_Extensions.OnRenderBegin();
				m_AppLayer->OnRender();
				m_Extensions.OnRenderEnd();
			}

			// UI 
			if (!m_Minimized)
			{
				HZ_PROFILE_SCOPE("UI");

				m_Extensions.OnUIBegin();
				m_AppLayer->OnUIRender();
				m_Extensions.OnUIEnd();
			}

			{
				HZ_PROFILE_SCOPE("Renderer::End");
				Renderer::EndFrame();
				m_Window->SwapBuffers();
				m_Window->PollEvents();
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return true;
		}

		Renderer::Recreate(e.GetWidth(), e.GetHeight(), m_Window->IsVSync());

		m_Minimized = false;
		return false;
	}

}
