#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Entrypoint.hpp>

// Extensions
#include "../Extensions/ImGui/ImGuiExtension.hpp"

// Main layer
#include "MainLayer.hpp"

class Sandbox : public Hz::Application
{
public:
	Sandbox(const Hz::ApplicationSpecification& appInfo)
		: Hz::Application(appInfo, "Sandbox")
	{
		SetAppLayer<MainLayer>();
		AddExtension<ImGuiExtension>();

		Init();
	}
};



// ----------------------------------------------------------------
//                    Set Application specs here...
// ----------------------------------------------------------------
Hz::Application* Hz::CreateApplication(int argc, char* argv[])
{
	ApplicationSpecification appInfo = {};
	appInfo.WindowSpecs.Title = "Sandbox | Horizon Application";
	appInfo.WindowSpecs.Width = 1280;
	appInfo.WindowSpecs.Height = 720;

	appInfo.RendererSpecs.Buffers = BufferCount::Triple;
	appInfo.RendererSpecs.VSync = false;

	return new Sandbox(appInfo);
}