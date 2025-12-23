// This header contains all of the basic utilities
// Types, macros, logger etc.

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace nbody {
// Type aliases for basic data types conforming with the design principles of the project
using U8    = std::uint8_t;
using U16   = std::uint16_t;
using U32   = std::uint32_t;
using U64   = std::uint64_t;
using USize = std::size_t;
using I8    = std::int8_t;
using I16   = std::int16_t;
using I32   = std::int32_t;
using I64   = std::int64_t;
using F32   = float;
using F64   = double;

enum class LoggerSeverity : U16 { DEBUG = 1, INFO = 1 << 1, WARNING = 1 << 2, ERROR = 1 << 3, FATAL = 1 << 4 };

enum class LoggerLayer : U16 { CORE = 1, RENDERER = 1 << 1, APP = 1 << 2 };

enum class LoggerTarget : U16 { CONSOLE = 1, FILE = 1 << 1 };

class LoggerInterface {
   public:
    enum class Color { RED, YELLOW, GREEN, BLUE, PURPLE, CYAN, WHITE };

   public:
    static void log(LoggerLayer layer, LoggerSeverity severity, const std::string& message);

    static std::span<LoggerLayer>    layers();
    static std::span<LoggerSeverity> severities();

    static void set_enabled_layers(const std::vector<LoggerLayer>& enabled_layers);
    static void set_enabled_severities(const std::vector<LoggerSeverity>& enabled_severities);

   private:
    static U16        m_severity_mask;
    static U16        m_layer_mask;
    static std::mutex m_mutex;

   private:
    static void impl_log_console(LoggerLayer layer, LoggerSeverity severity, const std::string& message);
    static void impl_log_file(LoggerLayer layer, LoggerSeverity severity, const std::string& message);

    static std::string impl_render_console_color_text(Color color, const std::string& text);
    static std::string impl_get_console_color_ansi_code(Color color);
    static std::string impl_get_current_timestamp();

    static U16 impl_get_layer_mask(const std::vector<LoggerLayer>& layers);
    static U16 impl_get_severity_mask(const std::vector<LoggerSeverity>& severities);

    static std::string impl_get_severity_name(LoggerSeverity severity);
    static std::string impl_get_layer_name(LoggerLayer layer);
};
}  // namespace nbody
