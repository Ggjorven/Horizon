#include <Horizon/Core/Application.hpp>
#include <Horizon/Core/Window.hpp>
#include <Horizon/Renderer/Renderer.hpp>

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

    while (!s_ShouldClose)
    {
        window->PollEvents();

        window->SwapBuffers();
    }

    return 0;
}
