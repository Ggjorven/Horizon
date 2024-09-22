#include "hzpch.h"
#include "Application.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Utils/Profiler.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <Pulse/Time/Timer.hpp>

namespace Hz
{

	Application* Application::s_Instance = nullptr;



	Application::Application(const ApplicationSpecification& appInfo, const std::string& debugName)
		: m_AppInfo(appInfo), m_DebugName(debugName)
	{
		s_Instance = this;

		Log::Init();
	}

	Application::~Application()
	{
		m_Extensions.OnDestroyBegin();

		m_AppLayer->OnDestroy();

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
				HZ_PROFILE_SCOPE("App::Update");

				m_Extensions.OnUpdateBegin(deltaTime);
				m_AppLayer->OnUpdate(deltaTime);
				m_Extensions.OnUpdateEnd(deltaTime);
			}

			if (!m_Minimized)
			{
				HZ_PROFILE_SCOPE("App::Render");

				m_Extensions.OnRenderBegin();
				m_AppLayer->OnRender();
				m_Extensions.OnRenderEnd();
			}

			// UI 
			if (!m_Minimized)
			{
				HZ_PROFILE_SCOPE("App::UI");

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

	void Application::Init()
	{
		m_Extensions.OnInitBegin();

		// This also intializes the renderer
		WindowSpecification windowSpecs = m_AppInfo.WindowSpecs;
		windowSpecs.EventCallback = [this](Event& e) { OnEvent(e); };
		m_Window = Window::Create(windowSpecs, m_AppInfo.RendererSpecs);

		m_AppLayer->OnInit();

		m_Extensions.OnInitEnd();

		// Log information about the application
		#if !defined(HZ_CONFIG_DIST)
			DeviceSpecification gpuInfo = DeviceSpecification::Get();

			HZ_LOG_INFO("Started \"{0}\" | Horizon Application", m_DebugName);
			HZ_LOG_INFO("  - GPU Information: ");
			HZ_LOG_INFO("    - Name: {0}", gpuInfo.DeviceName);
			HZ_LOG_INFO("    - Type: {0}", gpuInfo.DeviceType);
			HZ_LOG_INFO("    - API Version: {0}", gpuInfo.APIVersion);
			if (!m_Extensions.Empty())
				HZ_LOG_INFO("  - Loaded extensions: ");
			for (const auto& extension : m_Extensions)
				HZ_LOG_INFO("    - \"{0}\"", extension->GetName());
		#endif
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
