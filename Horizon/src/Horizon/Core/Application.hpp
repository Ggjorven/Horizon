#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

#include "Horizon/Core/Window.hpp"
#include "Horizon/Core/Layer.hpp"
#include "Horizon/Core/Extension.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"

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
		Application(const ApplicationSpecification& appInfo);
		~Application();

		void OnEvent(Event& e);

		void Run();

		// App layers & extension layers
		template<typename TLayer> void SetAppLayer() { m_AppLayer = Unique<TLayer>::Create(); }
		template<typename TExtension> void AddExtension() { m_Extensions.Add<TExtension>(); }

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
		ApplicationSpecification m_AppInfo = {};

		Ref<Window> m_Window = nullptr;
		bool m_Running = true;
		bool m_Minimized = false;

		Unique<Layer> m_AppLayer = nullptr;
		ExtensionList m_Extensions = {};

	private:
		static Application* s_Instance;

	};



	// Implemented by USER/CLIENT
	Application* CreateApplication(int argc, char* argv[]);

}