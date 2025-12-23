// This header contains all of the basic utilities
// Types, macros, logger etc.

#include <atomic>
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

enum class LoggerSeverity : U16 { INFO = 1, WARNING = 1 << 1, ERROR = 1 << 2, FATAL = 1 << 3 };

enum class LoggerLayer : U16 { CORE = 1, RENDERER = 1 << 1, APP = 1 << 2 };

class Logger {
    // Type definitions
   public:
    enum class Color { RED, YELLOW, GREEN, BLUE, PURPLE, CYAN, WHITE };

    // Funcionality
   public:
    static void log(LoggerLayer layer, LoggerSeverity severity, const std::string& message);

    static void init();
    static void init(std::vector<LoggerLayer> enabled_layers, std::vector<LoggerSeverity> enabled_severities);
    static void init(std::vector<LoggerLayer> enabled_layers, std::vector<LoggerSeverity> enabled_severities,
                     std::unordered_map<LoggerSeverity, Color>);

    static std::span<LoggerLayer>    enabled_layers();
    static std::span<LoggerSeverity> enabled_severities();

    // Note: Overrides layers and severities
    // Unsafe: This operation should only be used in extreme cases where performance is critical
    static void unsafe_set_enabled_layers(const std::vector<LoggerLayer>& enabled_layers);
    static void unsafe_set_enabled_severities(const std::vector<LoggerSeverity>& enabled_severities);

    // Constructors, destructors, assignment operators
   private:
    Logger()                         = default;
    ~Logger()                        = default;
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    // Member variables
   private:
    static std::atomic<U16>            m_severity_mask;
    static std::atomic<U16>            m_layer_mask;
    static std::vector<LoggerLayer>    m_enabled_layers;
    static std::vector<LoggerSeverity> m_enabled_severities;

    // Note: Mutex for thread safety
    static std::mutex m_mutex;

    static std::unordered_map<LoggerSeverity, Color> m_severity_colors_table;

    // Helper functions
   private:
    static U16 impl_get_layers_mask(const std::vector<LoggerLayer>& layers);
    static U16 impl_get_severities_mask(const std::vector<LoggerSeverity>& severities);

    static std::string impl_get_severity_name(LoggerSeverity severity);
    static std::string impl_get_layer_name(LoggerSeverity severity);

    static std::string impl_render_terminal_color_text(const std::string& text);
};
}  // namespace nbody
