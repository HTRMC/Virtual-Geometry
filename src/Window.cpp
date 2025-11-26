#include "Window.hpp"
#include "Logger.hpp"
#include <tracy/Tracy.hpp>
#include <format>

auto Window::create(const std::string& title, uint32_t width, uint32_t height)
    -> Result<Window> {
    ZoneScoped;
    Window window(title, width, height);

    if (auto result = window.initialize(title); !result) {
        return std::unexpected(result.error());
    }

    return window;
}

Window::Window(const std::string& title, uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {}

auto Window::initialize(const std::string& title) noexcept -> VoidResult {
    ZoneScoped;
    if (!glfwInit()) {
        return std::unexpected(makeError(
            ErrorCode::InitializationFailed,
            "Failed to initialize GLFW"
        ));
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* rawWindow = glfwCreateWindow(
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        title.c_str(),
        nullptr,
        nullptr
    );

    if (!rawWindow) {
        glfwTerminate();
        return std::unexpected(makeError(
            ErrorCode::WindowCreationFailed,
            "Failed to create GLFW window"
        ));
    }

    m_window.reset(rawWindow);

    glfwSetWindowUserPointer(m_window.get(), this);
    glfwSetFramebufferSizeCallback(m_window.get(), framebufferResizeCallback);

    Logger::info("Window created: {}x{}", m_width, m_height);
    return {};
}

Window::~Window() {
    ZoneScoped;
    // m_window automatically cleaned up by unique_ptr deleter
    if (m_window) {
        Logger::info("Window destroyed");
    }
}

auto Window::shouldClose() const noexcept -> bool {
    return glfwWindowShouldClose(m_window.get());
}

void Window::pollEvents() const noexcept {
    ZoneScoped;
    glfwPollEvents();
}

auto Window::getAspectRatio() const noexcept -> std::optional<float> {
    if (m_height == 0) {
        return std::nullopt;
    }
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

auto Window::getRequiredExtensions() const -> std::vector<const char*> {
    ZoneScoped;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Check if Vulkan is available
    if (!glfwExtensions) {
        return {};
    }

    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    ZoneScoped;
    auto* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));

    // Validate window pointer
    if (!windowPtr) {
        return;
    }

    // Validate dimensions - GLFW can pass negative or zero values during minimization
    if (width <= 0 || height <= 0) {
        Logger::debug("Window minimized or invalid dimensions: {}x{}", width, height);
        windowPtr->m_framebufferResized = true;
        return;
    }

    windowPtr->m_width = static_cast<uint32_t>(width);
    windowPtr->m_height = static_cast<uint32_t>(height);
    windowPtr->m_framebufferResized = true;

    if (windowPtr->m_resizeCallback) {
        windowPtr->m_resizeCallback(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        );
    }

    Logger::debug("Window resized: {}x{}", width, height);
}