#include "base.hpp"

#include <chrono>
#include <cstring>
#include <format>
#include <mutex>
#include <string>

// Windows compile-time optimizations
// Includes only necessary headers
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

std::string log_layer_to_string(LogLayer layer) {
    switch (layer) {
        case LogLayer::BASE:
            return "BASE";
        case LogLayer::RENDERER:
            return "RENDERER";
        case LogLayer::PHYSICS:
            return "PHYSICS";
        case LogLayer::APP:
            return "APP";
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

    if (!use_ansi) return text;

    const std::string code  = impl_get_console_color_ansi_code(color);
    const std::string reset = "\x1b[0m";

    return code + text + reset;
}

std::string LoggerInterface::impl_get_console_color_ansi_code(LogColor color) {
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

std::string LoggerInterface::impl_get_current_timestamp() {
    using namespace std::chrono;

    time_point<system_clock, seconds> const now_sec   = floor<seconds>(system_clock::now());
    local_time<seconds> const               now_local = current_zone()->to_local(now_sec);
    return std::format("{:%Y-%m-%d %H:%M:%S}", now_local);
}

}  // namespace nbody
