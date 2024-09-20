#pragma once

// To be defined by user/client.
extern Hz::Application* Hz::CreateApplication(int argc, char* argv[]);

namespace Hz
{
	int Main(int argc, char* argv[])
	{
		Hz::Application* app = Hz::CreateApplication(argc, argv);
		app->Run();
		delete app;
		return 0;
	}
}

#if defined(HZ_PLATFORM_WINDOWS)
	#if defined(HZ_CONFIG_DIST)
		#include <Windows.h>

		int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
		{
			return Hz::Main(__argc, __argv);
		}
	#else
		int main(int argc, char* argv[])
		{
			return Hz::Main(argc, argv);
		}
	#endif
#elif defined(HZ_PLATFORM_LINUX)
	int main(int argc, char* argv[])
	{
		return Hz::Main(argc, argv);
	}
#elif defined(HZ_PLATFORM_MACOS)
	int main(int argc, char* argv[])
	{
		return Hz::Main(argc, argv);
	}
#endif