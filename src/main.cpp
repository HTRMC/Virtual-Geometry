#include "Application.hpp"
#include "Logger.hpp"

auto main() -> int {
    Application::Config config{
        .applicationName = "Virtual Geometry - Demo",
        .windowWidth = 1280,
        .windowHeight = 720,
        .enableValidationLayers = true
    };

    auto appResult = Application::create(config);

    if (!appResult) {
        Logger::critical("Application initialization failed: {}", appResult.error().toString());
        return 1;
    }

    return appResult->run();
}