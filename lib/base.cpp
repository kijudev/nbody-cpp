#include "base.hpp"

#include <atomic>
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
U16 Logger::m_severity_mask{0};
U16 Logger::m_layer_mask{0};
U16 Logger::m_target_mask{0};

std::vector<LoggerLayer>    Logger::m_enabled_layers{};
std::vector<LoggerSeverity> Logger::m_enabled_severities{};
std::vector<LoggerTarget>   Logger::m_enabled_targets{};

std::mutex Logger::m_mutex;

std::unordered_map<LoggerSeverity, Logger::Color> Logger::m_severity_colors_table = {
    {LoggerSeverity::DEBUG,   Logger::Color::CYAN  },
    {LoggerSeverity::INFO,    Logger::Color::GREEN },
    {LoggerSeverity::WARNING, Logger::Color::YELLOW},
    {LoggerSeverity::ERROR,   Logger::Color::RED   },
    {LoggerSeverity::FATAL,   Logger::Color::PURPLE},
};

// log
void Logger::log(LoggerLayer layer, LoggerSeverity severity, const std::string& message) {}

std::span<LoggerLayer> Logger::layers() {
    return std::span<LoggerLayer>(m_enabled_layers.data(), m_enabled_layers.size());
}

std::span<LoggerSeverity> Logger::severities() {
    return std::span<LoggerSeverity>(m_enabled_severities.data(), m_enabled_severities.size());
}

std::span<LoggerTarget> Logger::targets() {
    return std::span<LoggerTarget>(m_enabled_targets.data(), m_enabled_targets.size());
}

const std::unordered_map<LoggerSeverity, Logger::Color> Logger::severity_colors() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_severity_colors_table;
}

void Logger::set_enabled_layers(const std::vector<LoggerLayer>& enabled_layers) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_enabled_layers = enabled_layers;
    m_layer_mask     = impl_get_layer_mask(m_enabled_layers);
}

void Logger::set_enabled_severities(const std::vector<LoggerSeverity>& enabled_severities) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_enabled_severities = enabled_severities;
    m_severity_mask      = impl_get_severity_mask(m_enabled_severities);
}

void Logger::set_enabled_targets(const std::vector<LoggerTarget>& enabled_targets) {
    std::lock_guard<std::mutex> lk(m_mutex);

    m_enabled_targets = enabled_targets;
    m_target_mask     = impl_get_target_mask(m_enabled_targets);
}

void Logger::set_severity_colors(const std::unordered_map<LoggerSeverity, Color>& severity_colors) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_severity_colors_table = severity_colors;
}

U16 Logger::impl_get_layer_mask(const std::vector<LoggerLayer>& layers) {
    U16 mask = 0;
    for (LoggerLayer l : layers) {
        mask |= static_cast<U16>(l);
    }
    return mask;
}

U16 Logger::impl_get_severity_mask(const std::vector<LoggerSeverity>& severities) {
    U16 mask = 0;
    for (LoggerSeverity s : severities) {
        mask |= static_cast<U16>(s);
    }
    return mask;
}

U16 Logger::impl_get_target_mask(const std::vector<LoggerTarget>& targets) {
    U16 mask = 0;
    for (LoggerTarget t : targets) {
        mask |= static_cast<U16>(t);
    }
    return mask;
}

std::string Logger::impl_get_severity_name(LoggerSeverity severity) {
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

std::string Logger::impl_get_layer_name(LoggerLayer layer) {
    switch (layer) {
        case LoggerLayer::CORE:
            return "CORE";
        case LoggerLayer::RENDERER:
            return "RENDERER";
        case LoggerLayer::APP:
            return "APP";
    }
}

std::string Logger::impl_get_target_name(LoggerTarget target) {
    switch (target) {
        case LoggerTarget::CONSOLE:
            return "CONSOLE";
        case LoggerTarget::FILE:
            return "FILE";
    }
}

std::string Logger::impl_render_console_color_text(Color color, const std::string& text) {
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

std::string Logger::impl_get_console_color_ansi_code(Color color) {
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

std::string Logger::impl_get_current_timestamp() {
    using namespace std::chrono;

    time_point<system_clock, seconds> const now_sec   = floor<seconds>(system_clock::now());
    local_time<seconds> const               now_local = current_zone()->to_local(now_sec);
    return std::format("{:%Y-%m-%d %H:%M:%S}", now_local);
}

}  // namespace nbody
