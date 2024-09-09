#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Renderer.hpp"

#include <cstdint>
#include <string>
#include <functional>

namespace Hz
{

    typedef std::function<void(Event&)> EventCallbackFn;

	struct WindowSpecification
	{
	public:
		uint32_t Width, Height;
		std::string Title;

		EventCallbackFn EventCallback;

	public:
		WindowSpecification(uint32_t width = 1280, uint32_t height = 720, const std::string& title = "Horizon Window", EventCallbackFn eventCallback = nullptr)
			: Width(width), Height(height), Title(title), EventCallback(eventCallback) {}
		~WindowSpecification() = default;
	};

    // Note: The window class is implemented based on platform
    class Window : public RefCounted // Only a single window is supported at a time
	{
	public:
		Window(const WindowSpecification windowSpecs, const RendererSpecification rendererSpecs);
		virtual ~Window();

		void PollEvents();
		void SwapBuffers();

		void Close();
		// Dont' use in any type of EventCallback! + Make sure to not use any window function after this call
		void ForceClose();

		inline uint32_t GetWidth() const { return m_Specification.Width; }
		inline uint32_t GetHeight() const { return m_Specification.Height; }
		std::pair<float, float> GetPosition() const;

		// Note: Used for mobile development
		inline void SwapWidthAndHeight() { std::swap(m_Specification.Width, m_Specification.Height); }

		void SetVSync(bool vsync);
		void SetTitle(const std::string& title);

		inline bool IsVSync() const { return Renderer::GetSpecification().VSync; }
		inline bool IsOpen() const { return !m_Closed; }
        inline bool IsMinimized() const { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

		inline void* GetNativeWindow() { return m_Window; }

		inline WindowSpecification& GetSpecification() { return m_Specification; }

        static Window& Get();
        static Ref<Window> Create(const WindowSpecification windowSpecs, const RendererSpecification rendererSpecs);

	private:
		WindowSpecification m_Specification;
        bool m_Closed = false;

		void* m_Window = nullptr;
	};

}
