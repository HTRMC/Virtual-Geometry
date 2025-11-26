#include "Window.hpp"
#include "Logger.hpp"
#include <format>

auto Window::create(std::string_view title, uint32_t width, uint32_t height)
    -> Result<Window> {
    Window window(title, width, height);

    if (auto result = window.initialize(title); !result) {
        return std::unexpected(result.error());
    }

    return window;
}

Window::Window(std::string_view title, uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {}

auto Window::initialize(std::string_view title) noexcept -> VoidResult {
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
        title.data(),
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
    // m_window automatically cleaned up by unique_ptr deleter
    if (m_window) {
        glfwTerminate();
        Logger::info("Window destroyed");
    }
}

auto Window::shouldClose() const noexcept -> bool {
    return glfwWindowShouldClose(m_window.get());
}

void Window::pollEvents() const noexcept {
    glfwPollEvents();
}

auto Window::getAspectRatio() const noexcept -> float {
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

auto Window::getRequiredExtensions() const -> std::vector<const char*> {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(window));

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