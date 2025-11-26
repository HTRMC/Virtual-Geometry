#pragma once

#include <vulkan/vulkan.h>
#include "Error.hpp"
#include <string_view>
#include <vector>
#include <optional>

class Window;

class VulkanContext {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] constexpr auto isComplete() const noexcept -> bool {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    [[nodiscard]] static auto create(const Window& window, const std::string& appName, bool enableValidation)
        -> Result<VulkanContext>;
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&& other) noexcept;
    VulkanContext& operator=(VulkanContext&& other) noexcept;

    [[nodiscard]] auto beginFrame() -> std::optional<uint32_t>;
    void endFrame();
    void waitIdle() const;

    [[nodiscard]] auto getInstance() const noexcept -> VkInstance { return m_instance; }
    [[nodiscard]] auto getDevice() const noexcept -> VkDevice { return m_device; }
    [[nodiscard]] auto getPhysicalDevice() const noexcept -> VkPhysicalDevice { return m_physicalDevice; }
    [[nodiscard]] auto getGraphicsQueue() const noexcept -> VkQueue { return m_graphicsQueue; }
    [[nodiscard]] auto getPresentQueue() const noexcept -> VkQueue { return m_presentQueue; }

private:
    VulkanContext() = default;
    [[nodiscard]] auto initialize(const Window& window, const std::string& appName, bool enableValidation) noexcept -> VoidResult;

    [[nodiscard]] auto createInstance(const std::string& appName) noexcept -> VoidResult;
    [[nodiscard]] auto setupDebugMessenger() noexcept -> VoidResult;
    [[nodiscard]] auto createSurface(const Window& window) noexcept -> VoidResult;
    [[nodiscard]] auto pickPhysicalDevice() noexcept -> VoidResult;
    [[nodiscard]] auto createLogicalDevice() noexcept -> VoidResult;

    [[nodiscard]] auto checkValidationLayerSupport() const -> bool;
    [[nodiscard]] auto findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices;
    [[nodiscard]] auto isDeviceSuitable(VkPhysicalDevice device) const -> bool;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_presentQueue{VK_NULL_HANDLE};

    bool m_enableValidationLayers;
    std::vector<const char*> m_validationLayers{"VK_LAYER_KHRONOS_validation"};
};