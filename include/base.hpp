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

class Logger {
    // Type definitions
   public:
    enum class Color { RED, YELLOW, GREEN, BLUE, PURPLE, CYAN, WHITE };

    // Funcionality
   public:
    static void log(LoggerLayer layer, LoggerSeverity severity, const std::string& message);

    static void init(const std::vector<LoggerTarget>& targets, const std::vector<LoggerLayer>& enabled_layers,
                     const std::vector<LoggerSeverity>& enabled_severities);

    static std::span<LoggerLayer>                          layers();
    static std::span<LoggerSeverity>                       severities();
    static std::span<LoggerTarget>                         targets();
    static const std::unordered_map<LoggerSeverity, Color> severity_colors();

    // Note: Overrides layers and severities
    // Unsafe: This operation should only be used in extreme cases where performance is critical
    static void set_enabled_layers(const std::vector<LoggerLayer>& enabled_layers);
    static void set_enabled_severities(const std::vector<LoggerSeverity>& enabled_severities);
    static void set_enabled_targets(const std::vector<LoggerTarget>& enabled_targets);
    static void set_severity_colors(const std::unordered_map<LoggerSeverity, Color>& severity_colors);
    static void set_output_file(const std::string& output_file);

    // Constructors, destructors, assignment operators
   private:
    Logger()                         = default;
    ~Logger()                        = default;
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    // Member variables
   private:
    static U16 m_severity_mask;
    static U16 m_layer_mask;
    static U16 m_target_mask;

    static std::vector<LoggerLayer>    m_enabled_layers;
    static std::vector<LoggerSeverity> m_enabled_severities;
    static std::vector<LoggerTarget>   m_enabled_targets;

    // Note: Mutex for thread safety
    static std::mutex m_mutex;

    static std::unordered_map<LoggerSeverity, Color> m_severity_colors_table;

    // Helper functions
   private:
    static U16 impl_get_layer_mask(const std::vector<LoggerLayer>& layers);
    static U16 impl_get_severity_mask(const std::vector<LoggerSeverity>& severities);
    static U16 impl_get_target_mask(const std::vector<LoggerTarget>& targets);

    static std::string impl_get_severity_name(LoggerSeverity severity);
    static std::string impl_get_layer_name(LoggerLayer layer);
    static std::string impl_get_target_name(LoggerTarget target);

    static std::string impl_render_console_color_text(Color color, const std::string& text);
    static std::string impl_get_console_color_ansi_code(Color color);
    static std::string impl_get_current_timestamp();
};
}  // namespace nbody
