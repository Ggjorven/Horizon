#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

#include "Horizon/Core/Window.hpp"
#include "Horizon/Core/Layer.hpp"
#include "Horizon/Core/Extension.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

#include <Pulse/Types/TypeUtils.hpp>

#include <vector>
#include <memory>
#include <queue>

namespace Hz
{

	struct ApplicationSpecification
	{
	public:
		WindowSpecification WindowSpecs = { };
		RendererSpecification RendererSpecs = { };
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& appInfo, const std::string& debugName = "Default Horizon App");
		~Application();

		void OnEvent(Event& e);

		void Run();

		// App layers & extension layers
		template<typename TLayer> 
		void SetAppLayer() requires (Pulse::Types::InheritsFrom<Layer, TLayer>);
		template<typename TExtension> 
		void AddExtension() requires (Pulse::Types::InheritsFrom<Extension, TExtension>);

		inline void Close() { m_Running = false; }
		inline bool IsMinimized() const { return m_Minimized; }

		inline Window& GetWindow() { return *m_Window; }
		inline const ApplicationSpecification& GetSpecification() const { return m_AppInfo; }

		inline static Application& Get() { return *s_Instance; }

	protected:
		void Init();

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_AppInfo;
		std::string m_DebugName;

		Ref<Window> m_Window = nullptr;
		bool m_Running = true;
		bool m_Minimized = false;
		bool m_UI = false;

		Unique<Layer> m_AppLayer = nullptr;
		ExtensionList m_Extensions = {};

	private:
		static Application* s_Instance;
	};

	///////////////////////////////////////////////////////////
	// Templated functions
	///////////////////////////////////////////////////////////
	template<typename TLayer>
	void Application::SetAppLayer() requires (Pulse::Types::InheritsFrom<Layer, TLayer>)
	{ 
		m_AppLayer = Unique<TLayer>::Create(); 
	}

	template<typename TExtension>
	void Application::AddExtension() requires (Pulse::Types::InheritsFrom<Extension, TExtension>)
	{ 
		m_Extensions.Add<TExtension>();
	}



	// Implemented by USER/CLIENT
	Application* CreateApplication(int argc, char* argv[]);

}