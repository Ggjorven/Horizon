#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Entrypoint.hpp>

#include "MainLayer.hpp"

class Sandbox : public Hz::Application
{
public:
	Sandbox(const Hz::ApplicationSpecification& appInfo)
		: Hz::Application(appInfo)
	{
		SetAppLayer<MainLayer>();
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