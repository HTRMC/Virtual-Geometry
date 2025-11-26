#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

class Logger {
public:
    static void init();

    template<typename... Args>
    static void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        s_logger->critical(fmt, std::forward<Args>(args)...);
    }

    [[nodiscard]] static auto getLogger() noexcept -> std::shared_ptr<spdlog::logger> {
        return s_logger;
    }

private:
    inline static std::shared_ptr<spdlog::logger> s_logger;
};