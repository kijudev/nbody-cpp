#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "type.hpp"

namespace nbody::base {
using namespace nbody::base::type;

enum class LogColor {
    LOG_COLOR_RED,
    LOG_COLOR_YELLOW,
    LOG_COLOR_GREEN,
    LOG_COLOR_BLUE,
    LOG_COLOR_PURPLE,
    LOG_COLOR_CYAN,
    LOG_COLOR_WHITE,
};

// NOTE: Returns an ANSI code based on the log color.
std::string log_color_to_ansi_code(LogColor color);

// NOTE: Layers correspond to the different components of the application.
enum class LogLayer : U16 {
    LIB    = 1,
    GFX    = 1 << 1,
    APP    = 1 << 2,
    ASSERT = 1 << 3
};

// NOTE: Returns a string representation of the log layer; to be used in
// printing and debugging.
std::string log_layer_to_string(LogLayer layer);

// NOTE: Severities correspond to the different levels of logging.
enum class LogSeverity : U16 {
    DEBUG   = 1,
    INFO    = 1 << 1,
    WARNING = 1 << 2,
    ERROR   = 1 << 3,
    FATAL   = 1 << 4
};

// NOTE: Returns a string representation of the severity; to be used in printing
// and debugging.
std::string log_severity_to_string(LogSeverity severity);

// NOTE: Choose color based on severity.
LogColor log_severity_to_color(LogSeverity severity);

// NOTE: LoggerInterface is an abstract base class for logging implementations.
// NOTE: Thread-safe.
class LoggerInterface {
   public:
   public:
    virtual ~LoggerInterface()                   = default;
    virtual void log(LogLayer layer, LogSeverity severity,
                     const std::string& message) = 0;

    // NOTE: Enables logging.
    void enable();

    // NOTE: Disables logging.
    void disable();

    // NOTE: Thread-safe.
    // WARNING: Setting layers at runtime of the application is discouraged,
    // only recommended during initialization.
    void set_layers(const std::vector<LogLayer>& enabled_layers);

    // NOTE: Thread-safe.
    // WARNING: Setting severities at runtime of the application is discouraged,
    // only recommend during initialization.
    void set_severities(const std::vector<LogSeverity>& enabled_severities);

    // NOTE: Returns a vector of layers; layers are stored internally as a bit
    // mask.
    std::vector<LogLayer> layers();

    // NOTE: Returns a vector of currently active severities; severities are
    // stored internally as a bit mask.
    std::vector<LogSeverity> severities();

   protected:
    // WHY: Layers and severities are stored as bitmasks to allow for efficient
    // filtering and checking.
    U16 m_layer_mask{0};
    U16 m_severity_mask{0};

    // WHY: Mutex is used to ensure thread safety when accessing and modifying
    // the logger's state. It also helps prevent race conditions when multiple
    // threads attempt to log messages simultaneously.
    std::mutex m_mutex;
    bool       m_is_enabled{true};

   protected:
    static std::string s_format_console_color_text(LogColor           color,
                                                   const std::string& text);
    static std::string s_get_console_color_ansi_code(LogColor color);

    // TODO: Refactor into a general timestamp formatter.
    static std::string s_get_current_timestamp();
};

// NOTE: Outputs log messages to the console, uses colors by default.
// NOTE: Thread-safe.
class ConsoleLogger : public LoggerInterface {
   public:
    ConsoleLogger(bool is_color_enabled = true);

    void log(LogLayer layer, LogSeverity severity,
             const std::string& message) override;

   private:
    bool m_is_color_enabled{true};
};

// NOTE: Outputs log messages to a file, by default:
// `.logs/logs_YYY-MM-DD-HH-MM-SS.log` NOTE: Thread-safe.
class FileLogger : public LoggerInterface {
   public:
    FileLogger();
    FileLogger(const std::string& filename);
    ~FileLogger();

    void log(LogLayer layer, LogSeverity severity,
             const std::string& message) override;

   private:
    std::string   m_filename{};
    std::ofstream m_file;
};

// NOTE: Logger class is essentially a singleton that manages a collection of
// loggers. Use Logger::init() to initialize it. NOTE: Thread-safe.
class Logger {
   public:
    // NOTE: Logger class is a singleton. Initialize the logger with the default
    // loggers: ConsoleLogger, FileLogger. WARNING: Should be called only once.
    // TODO: Implement a mechanism to check if the logger has already been
    // initialized.
    static void init();

    // NOTE: Logger class is a singleton. Initialize the logger with custom
    // loggers.
    // WARNING: Should be called only once.
    // TODO: Implement a mechanism to check if the logger has already been
    // initialized.
    static void init(std::vector<std::unique_ptr<LoggerInterface>>&& loggers);

    // NOTE: Logger class is a singleton. Log a message with the specified layer
    // and severity.
    static void log(LogLayer layer, LogSeverity severity,
                    const std::string& message);

   private:
    // WHY: The rule of five is private to prevent copying and moving of the
    // singleton instance.

    Logger()                         = default;
    ~Logger()                        = default;
    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(const Logger&) = delete;

    static std::vector<std::unique_ptr<LoggerInterface>> m_loggers;

    // TODO: This is probably an overkill. See if it is actually needed.
    static std::mutex m_mutex;
    static bool       m_is_initialized;
};

}  // namespace nbody::base

// NOTE: Macros utilizing the global Logger singleton instance.
#if !defined(NDEBUG)
#define LOG_INIT_DEFAULT()             nbody::base::Logger::init()
#define LOG_INIT_WITH_LOGGERS(loggers) nbody::base::Logger::init(loggers)

#define LOG_LIB_DEBUG(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::LIB, \
                             nbody::base::LogSeverity::DEBUG, message)
#define LOG_LIB_INFO(message)                            \
    nbody::base::Logger::log(nbody::base::LogLayer::LIB, \
                             nbody::base::LogSeverity::INFO, message)
#define LOG_LIB_WARNING(message)                         \
    nbody::base::Logger::log(nbody::base::LogLayer::LIB, \
                             nbody::base::LogSeverity::WARNING, message)
#define LOG_LIB_ERROR(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::LIB, \
                             nbody::base::LogSeverity::ERROR, message)
#define LOG_LIB_FATAL(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::LIB, \
                             nbody::base::LogSeverity::FATAL, message)

#define LOG_GFX_DEBUG(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::GFX, \
                             nbody::base::LogSeverity::DEBUG, message)
#define LOG_GFX_INFO(message)                            \
    nbody::base::Logger::log(nbody::base::LogLayer::GFX, \
                             nbody::base::LogSeverity::INFO, message)
#define LOG_GFX_WARNING(message)                         \
    nbody::base::Logger::log(nbody::base::LogLayer::GFX, \
                             nbody::base::LogSeverity::WARNING, message)
#define LOG_GFX_ERROR(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::GFX, \
                             nbody::base::LogSeverity::ERROR, message)
#define LOG_GFX_FATAL(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::GFX, \
                             nbody::base::LogSeverity::FATAL, message)

#define LOG_APP_DEBUG(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::APP, \
                             nbody::base::LogSeverity::DEBUG, message)
#define LOG_APP_INFO(message)                            \
    nbody::base::Logger::log(nbody::base::LogLayer::APP, \
                             nbody::base::LogSeverity::INFO, message)
#define LOG_APP_WARNING(message)                         \
    nbody::base::Logger::log(nbody::base::LogLayer::APP, \
                             nbody::base::LogSeverity::WARNING, message)
#define LOG_APP_ERROR(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::APP, \
                             nbody::base::LogSeverity::ERROR, message)
#define LOG_APP_FATAL(message)                           \
    nbody::base::Logger::log(nbody::base::LogLayer::APP, \
                             nbody::base::LogSeverity::FATAL, message)
#else
#define LOG_INIT_DEFAULT()      ((void)0)
#define LOG_INIT_WITH_LOGGERS() ((void)0)

#define LOG_LIB_DEBUG(message) ((void)0)
#define LOG_LIB_INFO(message)  ((void)0)
#define LOG_LIB_WARN(message)  ((void)0)
#define LOG_LIB_ERROR(message) ((void)0)
#define LOG_LIB_FATAL(message) ((void)0)

#define LOG_GFX_DEBUG(message) ((void)0)
#define LOG_GFX_INFO(message)  ((void)0)
#define LOG_GFX_WARN(message)  ((void)0)
#define LOG_GFX_ERROR(message) ((void)0)
#define LOG_GFX_FATAL(message) ((void)0)

#define LOG_APP_DEBUG(message) ((void)0)
#define LOG_APP_INFO(message)  ((void)0)
#define LOG_APP_WARN(message)  ((void)0)
#define LOG_APP_ERROR(message) ((void)0)
#define LOG_APP_FATAL(message) ((void)0)
#endif
