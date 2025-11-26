#pragma once

#include <expected>
#include <string>
#include <string_view>

enum class ErrorCode {
    None = 0,
    InitializationFailed,
    WindowCreationFailed,
    VulkanInstanceCreationFailed,
    VulkanDeviceNotFound,
    VulkanDeviceCreationFailed,
    SurfaceCreationFailed,
    ValidationLayersNotAvailable,
    DebugMessengerCreationFailed,
    Unknown
};

struct Error {
    ErrorCode code{ErrorCode::None};
    std::string message;

    Error() = default;
    Error(ErrorCode c, std::string_view msg) : code(c), message(msg) {}

    [[nodiscard]] constexpr auto isError() const noexcept -> bool {
        return code != ErrorCode::None;
    }

    [[nodiscard]] auto toString() const noexcept -> std::string_view {
        return message;
    }
};

template<typename T>
using Result = std::expected<T, Error>;

using VoidResult = std::expected<void, Error>;

// Helper to create error results
[[nodiscard]] inline auto makeError(ErrorCode code, std::string_view message) -> Error {
    return Error{code, message};
}