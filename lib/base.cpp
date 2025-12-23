#include "base.hpp"

#include <chrono>
#include <cstring>
#include <format>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

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

// Static members
U16 LoggerInterface::m_severity_mask{0};
U16 LoggerInterface::m_layer_mask{0};
U16 LoggerInterface::m_target_mask{0};

std::mutex LoggerInterface::m_mutex;

std::unordered_map<LoggerSeverity, LoggerInterface::Color> LoggerInterface::m_severity_colors_table = {
    {LoggerSeverity::DEBUG,   LoggerInterface::Color::CYAN  },
    {LoggerSeverity::INFO,    LoggerInterface::Color::GREEN },
    {LoggerSeverity::WARNING, LoggerInterface::Color::YELLOW},
    {LoggerSeverity::ERROR,   LoggerInterface::Color::RED   },
    {LoggerSeverity::FATAL,   LoggerInterface::Color::PURPLE},
};

// log
void LoggerInterface::log(LoggerLayer layer, LoggerSeverity severity, const std::string& message) {}

std::span<LoggerLayer> LoggerInterface::layers() { return {}; }

std::span<LoggerSeverity> LoggerInterface::severities() { return {}; }

void LoggerInterface::set_enabled_layers(const std::vector<LoggerLayer>& enabled_layers) {
    std::lock_guard<std::mutex> lk(m_mutex);
}

void LoggerInterface::set_enabled_severities(const std::vector<LoggerSeverity>& enabled_severities) {
    std::lock_guard<std::mutex> lk(m_mutex);
}

void LoggerInterface::set_severity_colors(const std::unordered_map<LoggerSeverity, Color>& severity_colors) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_severity_colors_table = severity_colors;
}

std::string LoggerInterface::impl_render_console_color_text(Color color, const std::string& text) {
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

std::string LoggerInterface::impl_get_console_color_ansi_code(Color color) {
    switch (color) {
        case Color::RED:
            return "\x1b[31m";
        case Color::YELLOW:
            return "\x1b[33m";
        case Color::GREEN:
            return "\x1b[32m";
        case Color::BLUE:
            return "\x1b[34m";
        case Color::PURPLE:
            return "\x1b[35m";
        case Color::CYAN:
            return "\x1b[36m";
        case Color::WHITE:
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

U16 LoggerInterface::impl_get_layer_mask(const std::vector<LoggerLayer>& layers) {
    U16 mask = 0;
    for (LoggerLayer l : layers) {
        mask |= static_cast<U16>(l);
    }
    return mask;
}

U16 LoggerInterface::impl_get_severity_mask(const std::vector<LoggerSeverity>& severities) {
    U16 mask = 0;
    for (LoggerSeverity s : severities) {
        mask |= static_cast<U16>(s);
    }
    return mask;
}

std::string LoggerInterface::impl_get_severity_name(LoggerSeverity severity) {
    switch (severity) {
        case LoggerSeverity::DEBUG:
            return "DEBUG";
        case LoggerSeverity::INFO:
            return "INFO";
        case LoggerSeverity::WARNING:
            return "WARNING";
        case LoggerSeverity::ERROR:
            return "ERROR";
        case LoggerSeverity::FATAL:
            return "FATAL";
    }
}

std::string LoggerInterface::impl_get_layer_name(LoggerLayer layer) {
    switch (layer) {
        case LoggerLayer::CORE:
            return "CORE";
        case LoggerLayer::RENDERER:
            return "RENDERER";
        case LoggerLayer::APP:
            return "APP";
    }
}

}  // namespace nbody
