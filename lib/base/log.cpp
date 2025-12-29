// A minimal but sufficient logging library.

#include "base/log.hpp"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "base/type.hpp"

// WHY: Windows compile-time optimizations. Includes only necessary headers.
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace nbody {
std::string log_color_ansi_code(LogColor color) {
    switch (color) {
        case LogColor::RED:
            return "\x1b[31m";
        case LogColor::YELLOW:
            return "\x1b[33m";
        case LogColor::GREEN:
            return "\x1b[32m";
        case LogColor::BLUE:
            return "\x1b[34m";
        case LogColor::PURPLE:
            return "\x1b[35m";
        case LogColor::CYAN:
            return "\x1b[36m";
        case LogColor::WHITE:
            return "\x1b[37m";
        default:
            return "\x1b[0m";
    }
}

std::string log_layer_to_string(LogLayer layer) {
    switch (layer) {
        case LogLayer::LIB:
            return "LIB";
        case LogLayer::GFX:
            return "GFX";
        case LogLayer::APP:
            return "APP";
        case LogLayer::ASSERT:
            return "ASSERT";
    }
}

std::string log_severity_to_string(LogSeverity severity) {
    switch (severity) {
        case LogSeverity::DEBUG:
            return "DEBUG";
        case LogSeverity::INFO:
            return "INFO";
        case LogSeverity::WARNING:
            return "WARNING";
        case LogSeverity::ERROR:
            return "ERROR";
        case LogSeverity::FATAL:
            return "FATAL";
    }
}

LogColor log_severity_to_color(LogSeverity severity) {
    switch (severity) {
        case nbody::LogSeverity::DEBUG:
            return LogColor::CYAN;
        case LogSeverity::INFO:
            return LogColor::GREEN;
        case LogSeverity::WARNING:
            return LogColor::YELLOW;
        case LogSeverity::ERROR:
            return LogColor::RED;
        case LogSeverity::FATAL:
            return LogColor::PURPLE;
    }
}

void LoggerInterface::enable() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_is_enabled = true;
}

void LoggerInterface::disable() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_is_enabled = false;
}

void LoggerInterface::set_layers(const std::vector<LogLayer>& enabled_layers) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_layer_mask = 0;
    for (LogLayer layer : enabled_layers) {
        m_layer_mask |= (1 << static_cast<U16>(layer));
    }
}

void LoggerInterface::set_severities(const std::vector<LogSeverity>& enabled_severities) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_severity_mask = 0;
    for (LogSeverity severity : enabled_severities) {
        m_severity_mask |= (1 << static_cast<U16>(severity));
    }
}

std::vector<LogLayer> LoggerInterface::layers() {
    std::lock_guard<std::mutex> lk(m_mutex);
    std::vector<LogLayer>       layers;

    for (U16 i = 0; i < 32; ++i) {
        if ((m_layer_mask & (1 << i)) != 0) {
            layers.push_back(static_cast<LogLayer>(i));
        }
    }

    return layers;
}

std::vector<LogSeverity> LoggerInterface::severities() {
    std::lock_guard<std::mutex> lk(m_mutex);
    std::vector<LogSeverity>    severities;

    for (U16 i = 0; i < 32; ++i) {
        if ((m_severity_mask & (1 << i)) != 0) {
            severities.push_back(static_cast<LogSeverity>(i));
        }
    }

    return severities;
}

std::string LoggerInterface::impl_format_console_color_text(LogColor           color,
                                                            const std::string& text) {
// NOTE: Windows; initialize VT mode if supported.
#if defined(_WIN32)
    static bool vt_inited = false;
    static bool vt_ok     = false;
    if (!vt_inited) {
        vt_inited   = true;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                const DWORD vt_flag = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if ((mode & vt_flag) == 0) {
                    vt_ok = (SetConsoleMode(hOut, mode | vt_flag) != 0);
                } else {
                    vt_ok = true;
                }
            }
        }
    }
    const bool use_ansi = vt_ok;
#else
    const bool use_ansi = true;
#endif

    // NOTE: Fallback to non-colored output if VT mode is not supported.
    if (!use_ansi) return text;

    const std::string code  = log_color_ansi_code(color);
    const std::string reset = "\x1b[0m";

    return code + text + reset;
}

std::string LoggerInterface::impl_get_current_timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm     tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

ConsoleLogger::ConsoleLogger(bool is_color_enabled)
    : LoggerInterface(), m_is_color_enabled(is_color_enabled) {}

void ConsoleLogger::log(LogLayer layer, LogSeverity severity, const std::string& message) {
    std::lock_guard<std::mutex> lk(m_mutex);

    if (!m_is_color_enabled) {
        std::cout << "[" << impl_get_current_timestamp() << "] [" << log_layer_to_string(layer)
                  << "] [" << log_severity_to_string(severity) << "]: " << message << ";\n";
    } else {
        std::cout << "[" << impl_get_current_timestamp() << "] "
                  << impl_format_console_color_text(log_severity_to_color(severity),
                                                    "[" + log_layer_to_string(layer) + "] [" +
                                                        log_severity_to_string(severity) + "]: ")
                  << message << ";\n";
    }
}

FileLogger::FileLogger() : LoggerInterface() {
    std::time_t t = std::time(nullptr);
    std::tm     tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", &tm);

    std::string filename = "./.logs/log_" + std::string(buf) + ".log";

    m_filename = filename;
    m_file.open(m_filename, std::ios::out | std::ios::app);
}

FileLogger::FileLogger(const std::string& filename) : LoggerInterface(), m_filename(filename) {
    m_file.open(m_filename, std::ios::out | std::ios::app);
}

FileLogger::~FileLogger() { m_file.close(); }

void FileLogger::log(LogLayer layer, LogSeverity severity, const std::string& message) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_file << "[" << impl_get_current_timestamp() << "] [" << log_layer_to_string(layer) << "] ["
           << log_severity_to_string(severity) << "]: " << message << ";\n";
}

std::vector<std::unique_ptr<LoggerInterface>> Logger::m_loggers{};
std::mutex                                    Logger::m_mutex{};
bool                                          Logger::m_is_initialized{false};

void Logger::init() {
    m_loggers.emplace_back(std::make_unique<ConsoleLogger>());
    m_loggers.emplace_back(std::make_unique<FileLogger>());

    m_is_initialized = true;
}

void Logger::init(std::vector<std::unique_ptr<LoggerInterface>>&& loggers) {
    for (auto& logger : loggers) {
        m_loggers.emplace_back(std::move(logger));
    }

    m_is_initialized = true;
}

void Logger::log(LogLayer layer, LogSeverity severity, const std::string& message) {
    std::lock_guard<std::mutex> lk(m_mutex);

    if (!m_is_initialized) {
        init();
    }

    for (auto& logger : m_loggers) {
        logger->log(layer, severity, message);
    }
}

}  // namespace nbody
