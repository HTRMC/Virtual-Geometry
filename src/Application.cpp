#include "Application.hpp"
#include "Window.hpp"
#include "VulkanContext.hpp"
#include "Logger.hpp"
#include <chrono>

auto Application::create(const Config& config) -> Result<Application> {
    Application app;

    if (auto result = app.initialize(config); !result) {
        return std::unexpected(result.error());
    }

    return app;
}

auto Application::initialize(const Config& config) -> VoidResult {
    Logger::init();
    Logger::info("Initializing application: {}", config.applicationName);

    auto windowResult = Window::create(
        config.applicationName,
        config.windowWidth,
        config.windowHeight
    );

    if (!windowResult) {
        return std::unexpected(windowResult.error());
    }

    m_window = std::make_unique<Window>(std::move(*windowResult));

    auto vulkanResult = VulkanContext::create(
        *m_window,
        config.applicationName,
        config.enableValidationLayers
    );

    if (!vulkanResult) {
        return std::unexpected(vulkanResult.error());
    }

    m_vulkanContext = std::make_unique<VulkanContext>(std::move(*vulkanResult));

    Logger::info("Application initialized successfully");
    return {};
}

Application::~Application() {
    Logger::info("Shutting down application");
    shutdown();
}

auto Application::run() -> int {
    Logger::info("Starting main loop");

    mainLoop();
    return 0;
}

void Application::mainLoop() {
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<float>;

    auto lastFrameTime = Clock::now();

    while (m_isRunning && !m_window->shouldClose()) {
        const auto currentTime = Clock::now();
        const float deltaTime = Duration(currentTime - lastFrameTime).count();
        lastFrameTime = currentTime;

        m_window->pollEvents();
        update(deltaTime);
        render();
    }

    m_vulkanContext->waitIdle();
}

void Application::update([[maybe_unused]] float deltaTime) {
    // Update logic will go here
}

void Application::render() {
    if (auto result = m_vulkanContext->beginFrame(); result) {
        // Render commands will go here
        m_vulkanContext->endFrame();
    }
}

void Application::shutdown() noexcept {
    m_isRunning = false;

    if (m_vulkanContext) {
        m_vulkanContext->waitIdle();
    }
}