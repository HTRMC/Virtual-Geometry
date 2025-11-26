#pragma once

#include "Error.hpp"
#include <memory>
#include <string_view>

class Window;
class VulkanContext;

class Application {
public:
    struct Config {
        std::string applicationName{"Virtual Geometry"};
        uint32_t windowWidth{1280};
        uint32_t windowHeight{720};
        bool enableValidationLayers{true};
    };

    [[nodiscard]] static auto create(const Config& config) -> Result<Application>;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept = default;
    Application& operator=(Application&&) noexcept = default;

    [[nodiscard]] auto run() -> int;
    void shutdown() noexcept;

private:
    Application() = default;
    [[nodiscard]] auto initialize(const Config& config) -> VoidResult;

    void mainLoop();
    void update(float deltaTime);
    void render();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<VulkanContext> m_vulkanContext;
    bool m_isRunning{true};
};