#include "base.hpp"

#include <cstring>
#include <string>

// Include necessary headers for Windows console handling
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace nbody {
void Logger::log(LoggerLayer layer, LoggerSeverity severity, const std::string& message) {}

U16 Logger::impl_get_layers_mask(const std::vector<LoggerLayer>& layers) {
    U16 mask = 0;
    for (LoggerLayer layer : layers) {
        mask |= static_cast<U16>(layer);
    }
    return mask;
}

U16 Logger::impl_get_severities_mask(const std::vector<LoggerSeverity>& severities) {
    U16 mask = 0;
    for (LoggerSeverity severity : severities) {
        mask |= static_cast<U16>(severity);
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
            return "INFO";
        case LoggerLayer::APP:
            return "WARNING";
        case LoggerLayer::RENDERER:
            return "ERROR";
    }
}

std::string Logger::impl_render_terminal_color_text(Color color, const std::string& text) {
    // On POSIX use ANSI escape sequences.
    // On Windows, attempt to enable Virtual Terminal Processing (VT) so ANSI escapes are interpreted.
#if defined(_WIN32)
    static bool vt_initialized = false;
    static bool vt_enabled     = false;
    if (!vt_initialized) {
        vt_initialized = true;
        // Try to enable VT processing on the console output handle.
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                // Try to set ENABLE_VIRTUAL_TERMINAL_PROCESSING
                const DWORD vt_flag = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if ((mode & vt_flag) == 0) {
                    if (SetConsoleMode(hOut, mode | vt_flag)) {
                        vt_enabled = true;
                    } else {
                        vt_enabled = false;
                    }
                } else {
                    vt_enabled = true;
                }
            }
        }
    }
#endif

    // Map our Color enum to ANSI color codes.
    const char* prefix = "";
    const char* suffix = "";
#if defined(_WIN32)
    const bool use_ansi = vt_enabled;
#else
    const bool use_ansi = true;
#endif

    if (use_ansi) {
        switch (color) {
            case Color::RED:
                prefix = "\x1b[31m";
                break;
            case Color::YELLOW:
                prefix = "\x1b[33m";
                break;
            case Color::GREEN:
                prefix = "\x1b[32m";
                break;
            case Color::BLUE:
                prefix = "\x1b[34m";
                break;
            case Color::PURPLE:
                prefix = "\x1b[35m";
                break;
            case Color::CYAN:
                prefix = "\x1b[36m";
                break;
            case Color::WHITE:
                prefix = "\x1b[37m";
                break;
            default:
                prefix = "\x1b[0m";
                break;
        }
        suffix = "\x1b[0m";
    }

    if (!use_ansi || prefix[0] == '\0') {
        return text;
    }

    std::string out;
    out.reserve(std::strlen(prefix) + text.size() + std::strlen(suffix));
    out += prefix;
    out += text;
    out += suffix;
    return out;
}

}  // namespace nbody
