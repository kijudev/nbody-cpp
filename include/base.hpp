// This header contains all of the basic utilities
// Types, macros, logger etc.

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <span>
#include <string>
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

enum class LogSeverity : U16 {
    DEBUG   = 1,
    INFO    = 1 << 1,
    WARNING = 1 << 2,
    ERROR   = 1 << 3,
    FATAL   = 1 << 4
};
std::string log_severity_to_string(LogSeverity severity);

enum class LogLayer : U16 { BASE = 1, RENDERER = 1 << 1, PHYSICS = 1 << 2, APP = 1 << 3 };
std::string log_layer_to_string(LogLayer layer);

enum class LogColor { RED, YELLOW, GREEN, BLUE, PURPLE, CYAN, WHITE };

class LoggerInterface {
   public:

   public:
    virtual void log(LogLayer layer, LogSeverity severity, const std::string& message) = 0;

    void enable();
    void disable();

    void set_layers(const std::vector<LogLayer>& enabled_layers);
    void set_severities(const std::vector<LogSeverity>& enabled_severities);

    std::vector<LogLayer>    layers();
    std::vector<LogSeverity> severities();

   private:
    U16        m_layer_mask{0};
    U16        m_severity_mask{0};
    std::mutex m_mutex;
    bool       m_is_enabled{true};

   private:
    static std::string impl_format_console_color_text(LogColor color, const std::string& text);
    static std::string impl_get_console_color_ansi_code(LogColor color);
    static std::string impl_get_current_timestamp();
};

class ConsoleLogger : private LoggerInterface {
   public:
    ConsoleLogger();

    void log(LogLayer layer, LogSeverity severity, const std::string& message) override;

   private:
    bool m_is_color_enabled{true};
};

class FileLogger : private LoggerInterface {
   public:
    FileLogger(const std::string& filename);

    void log(LogLayer layer, LogSeverity severity, const std::string& message) override;

   private:
    std::ofstream m_file{};
};

}  // namespace nbody
