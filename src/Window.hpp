#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Error.hpp"
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

struct GLFWwindowDeleter {
    void operator()(GLFWwindow* window) const noexcept {
        if (window) {
            glfwDestroyWindow(window);
        }
    }
};

using GLFWwindowPtr = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

class Window {
public:
    using ResizeCallback = std::function<void(uint32_t, uint32_t)>;

    [[nodiscard]] static auto create(const std::string& title, uint32_t width, uint32_t height)
        -> Result<Window>;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept = default;
    Window& operator=(Window&&) noexcept = default;

    [[nodiscard]] auto shouldClose() const noexcept -> bool;
    void pollEvents() const noexcept;

    [[nodiscard]] auto getWidth() const noexcept -> uint32_t { return m_width; }
    [[nodiscard]] auto getHeight() const noexcept -> uint32_t { return m_height; }
    [[nodiscard]] auto getAspectRatio() const noexcept -> std::optional<float>;
    [[nodiscard]] auto wasResized() const noexcept -> bool { return m_framebufferResized; }
    void resetResizedFlag() noexcept { m_framebufferResized = false; }

    [[nodiscard]] auto getHandle() const noexcept -> GLFWwindow* { return m_window.get(); }
    [[nodiscard]] auto getRequiredExtensions() const -> std::vector<const char*>;

    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = std::move(callback); }

private:
    Window(const std::string& title, uint32_t width, uint32_t height);
    [[nodiscard]] auto initialize(const std::string& title) noexcept -> VoidResult;
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindowPtr m_window;
    uint32_t m_width;
    uint32_t m_height;
    bool m_framebufferResized{false};
    ResizeCallback m_resizeCallback;
};