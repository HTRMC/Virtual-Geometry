#include "Logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

void Logger::init() {
    s_logger = spdlog::stdout_color_mt("APP");
    s_logger->set_pattern("%^[%T] [%l] %v%$");

#ifdef NDEBUG
    s_logger->set_level(spdlog::level::info);
#else
    s_logger->set_level(spdlog::level::trace);
#endif

    spdlog::set_default_logger(s_logger);
}