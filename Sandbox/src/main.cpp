#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Logging.hpp>
#include <Horizon/Core/Window.hpp>
#include <Horizon/Renderer/Renderer.hpp>

#include <Pulse/Time/Timer.hpp>

using namespace Hz;

static bool s_ShouldClose = false;
static void EventCallback(Event& e)
{
    EventHandler handler(e);
    handler.Handle<WindowResizeEvent>([](WindowResizeEvent& e) -> bool
    {
        Renderer::Recreate(e.GetWidth(), e.GetHeight(), Renderer::GetSpecification().VSync);
        return false;
    });
    handler.Handle<WindowCloseEvent>([](WindowCloseEvent& e) -> bool
    {
        s_ShouldClose = true;
        return false;
    });
}

int main(int argc, char* argv[])
{
    Application application = {};

    WindowSpecification windowSpecs = { 1280, 720, "Window", &EventCallback };
    RendererSpecification rendererSpecs = { BufferCount::Triple, false };
    Ref<Window> window = Window::Create(windowSpecs, rendererSpecs);

    Pulse::Time::Timer deltaTimer = {};
    Pulse::Time::Timer intervalTimer = {};
    double interval = 0.300; // Seconds
    uint32_t FPS = 0;

    while (!s_ShouldClose)
    {
        deltaTimer.Reset();

        window->PollEvents();
        Renderer::BeginFrame();



        Renderer::EndFrame();
        window->SwapBuffers();

        // Update FPS
        FPS++;
        if (intervalTimer.ElapsedSeconds() >= interval)
        {
            HZ_LOG_TRACE("Deltatime: {0:.4f}ms | FPS: {1} ({2} / {3:.5f})", deltaTimer.ElapsedMillis(), static_cast<uint32_t>(FPS / intervalTimer.ElapsedSeconds()), FPS, intervalTimer.ElapsedSeconds());

            intervalTimer.Reset();
            FPS = 0;
        }
    }

    return 0;
}
