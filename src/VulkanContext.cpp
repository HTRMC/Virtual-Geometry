#include "VulkanContext.hpp"
#include "Window.hpp"
#include "Logger.hpp"
#include <set>
#include <format>

auto VulkanContext::create(const Window& window, std::string_view appName, bool enableValidation)
    -> Result<VulkanContext> {
    VulkanContext context;
    context.m_enableValidationLayers = enableValidation;

    if (auto result = context.initialize(window, appName, enableValidation); !result) {
        return std::unexpected(result.error());
    }

    return context;
}

auto VulkanContext::initialize(const Window& window, std::string_view appName, bool enableValidation) noexcept
    -> VoidResult {
    Logger::info("Initializing Vulkan context");

    if (auto result = createInstance(appName); !result) {
        return std::unexpected(result.error());
    }

    if (auto result = setupDebugMessenger(); !result) {
        return std::unexpected(result.error());
    }

    if (auto result = createSurface(window); !result) {
        return std::unexpected(result.error());
    }

    if (auto result = pickPhysicalDevice(); !result) {
        return std::unexpected(result.error());
    }

    if (auto result = createLogicalDevice(); !result) {
        return std::unexpected(result.error());
    }

    Logger::info("Vulkan context initialized successfully");
    return {};
}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
    : m_instance(other.m_instance)
    , m_debugMessenger(other.m_debugMessenger)
    , m_surface(other.m_surface)
    , m_physicalDevice(other.m_physicalDevice)
    , m_device(other.m_device)
    , m_graphicsQueue(other.m_graphicsQueue)
    , m_presentQueue(other.m_presentQueue)
    , m_enableValidationLayers(other.m_enableValidationLayers)
    , m_validationLayers(std::move(other.m_validationLayers)) {

    // Nullify moved-from object to prevent double-free
    other.m_instance = VK_NULL_HANDLE;
    other.m_debugMessenger = VK_NULL_HANDLE;
    other.m_surface = VK_NULL_HANDLE;
    other.m_physicalDevice = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;
    other.m_graphicsQueue = VK_NULL_HANDLE;
    other.m_presentQueue = VK_NULL_HANDLE;
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept {
    if (this != &other) {
        // Clean up current resources
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
        }

        if (m_enableValidationLayers && m_debugMessenger != VK_NULL_HANDLE) {
            auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT")
            );
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }

        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        }

        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
        }

        // Transfer ownership
        m_instance = other.m_instance;
        m_debugMessenger = other.m_debugMessenger;
        m_surface = other.m_surface;
        m_physicalDevice = other.m_physicalDevice;
        m_device = other.m_device;
        m_graphicsQueue = other.m_graphicsQueue;
        m_presentQueue = other.m_presentQueue;
        m_enableValidationLayers = other.m_enableValidationLayers;
        m_validationLayers = std::move(other.m_validationLayers);

        // Nullify moved-from object
        other.m_instance = VK_NULL_HANDLE;
        other.m_debugMessenger = VK_NULL_HANDLE;
        other.m_surface = VK_NULL_HANDLE;
        other.m_physicalDevice = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
        other.m_graphicsQueue = VK_NULL_HANDLE;
        other.m_presentQueue = VK_NULL_HANDLE;
    }
    return *this;
}

VulkanContext::~VulkanContext() {
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
    }

    if (m_enableValidationLayers && m_debugMessenger != VK_NULL_HANDLE) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT")
        );
        if (func != nullptr) {
            func(m_instance, m_debugMessenger, nullptr);
        }
    }

    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
    }

    Logger::info("Vulkan context destroyed");
}

auto VulkanContext::createInstance(std::string_view appName) noexcept -> VoidResult {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        return std::unexpected(makeError(
            ErrorCode::ValidationLayersNotAvailable,
            "Validation layers requested but not available"
        ));
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        return std::unexpected(makeError(
            ErrorCode::VulkanInstanceCreationFailed,
            "Failed to create Vulkan instance"
        ));
    }

    Logger::info("Vulkan instance created");
    return {};
}

auto VulkanContext::setupDebugMessenger() noexcept -> VoidResult {
    if (!m_enableValidationLayers) {
        return {};
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT")
    );

    if (func == nullptr || func(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        return std::unexpected(makeError(
            ErrorCode::DebugMessengerCreationFailed,
            "Failed to set up debug messenger"
        ));
    }

    Logger::info("Debug messenger set up");
    return {};
}

auto VulkanContext::createSurface(const Window& window) noexcept -> VoidResult {
    if (glfwCreateWindowSurface(m_instance, window.getHandle(), nullptr, &m_surface) != VK_SUCCESS) {
        return std::unexpected(makeError(
            ErrorCode::SurfaceCreationFailed,
            "Failed to create window surface"
        ));
    }
    Logger::info("Window surface created");
    return {};
}

auto VulkanContext::pickPhysicalDevice() noexcept -> VoidResult {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        return std::unexpected(makeError(
            ErrorCode::VulkanDeviceNotFound,
            "Failed to find GPUs with Vulkan support"
        ));
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        return std::unexpected(makeError(
            ErrorCode::VulkanDeviceNotFound,
            "Failed to find a suitable GPU"
        ));
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    Logger::info("Selected GPU: {}", properties.deviceName);

    return {};
}

auto VulkanContext::createLogicalDevice() noexcept -> VoidResult {
    auto indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        return std::unexpected(makeError(
            ErrorCode::VulkanDeviceCreationFailed,
            "Failed to create logical device"
        ));
    }

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);

    Logger::info("Logical device created");
    return {};
}

auto VulkanContext::checkValidationLayerSupport() const -> bool {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

auto VulkanContext::findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

auto VulkanContext::isDeviceSuitable(VkPhysicalDevice device) const -> bool {
    auto indices = findQueueFamilies(device);
    return indices.isComplete();
}

auto VulkanContext::beginFrame() -> std::optional<uint32_t> {
    // Frame rendering will be implemented here
    return std::nullopt;
}

void VulkanContext::endFrame() {
    // Frame end logic will be implemented here
}

void VulkanContext::waitIdle() const {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData
) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        Logger::warn("Vulkan validation: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}