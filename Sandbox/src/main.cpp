#include "CustomApp.hpp"

#include <Horizon/Core/Application.hpp>

using namespace Hz;

int main(int argc, char* argv[])
{
    Application application = {};

    CustomApp* app = new CustomApp();
    app->Run();
    delete app;

    return 0;
}
