#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base.hpp"

namespace nbody {

enum class LogColor { RED, YELLOW, GREEN, BLUE, PURPLE, CYAN, WHITE };

enum class LogLayer : U16 { BASE = 1, RENDERER = 1 << 1, APP = 1 << 2 };
std::string log_layer_to_string(LogLayer layer);

enum class LogSeverity : U16 { INFO = 1, WARNING = 1 << 1, ERROR = 1 << 2, FATAL = 1 << 3 };
std::string log_severity_to_string(LogSeverity severity);
LogColor    log_severity_to_color(LogSeverity severity);

class LoggerInterface {
   public:
   public:
    virtual ~LoggerInterface()                                                         = default;
    virtual void log(LogLayer layer, LogSeverity severity, const std::string& message) = 0;

    void enable();
    void disable();

    void set_layers(const std::vector<LogLayer>& enabled_layers);
    void set_severities(const std::vector<LogSeverity>& enabled_severities);

    std::vector<LogLayer>    layers();
    std::vector<LogSeverity> severities();

   protected:
    U16        m_layer_mask{0};
    U16        m_severity_mask{0};
    std::mutex m_mutex;
    bool       m_is_enabled{true};

   protected:
    static std::string impl_format_console_color_text(LogColor color, const std::string& text);
    static std::string impl_get_console_color_ansi_code(LogColor color);
    static std::string impl_get_current_timestamp();
};

class ConsoleLogger : public LoggerInterface {
   public:
    ConsoleLogger(bool is_color_enabled = true);

    void log(LogLayer layer, LogSeverity severity, const std::string& message) override;

   private:
    bool m_is_color_enabled{true};
};

class FileLogger : public LoggerInterface {
   public:
    FileLogger();
    FileLogger(const std::string& filename);
    ~FileLogger();

    void log(LogLayer layer, LogSeverity severity, const std::string& message) override;

   private:
    std::string   m_filename{};
    std::ofstream m_file;
};

// Note: Logger class is essentially a singleton that manages a collection of loggers.
class Logger {
   public:
    // Note: Logger class is a singleton, use Logger::init() to initialize it.
    // Note: Initialize the logger with the default loggers; ConsoleLogger and FileLogger
    static void init();

    // Note: Logger class is a singleton, use
    // Logger::init(std::vector<std::unique_ptr<LoggerInterface>>&& loggers) to initialize it. Note:
    // Initialize the logger with custom loggers
    static void init(std::vector<std::unique_ptr<LoggerInterface>>&& loggers);

    // Note: Logger class is a singleton, use Logger::log(LogLayer layer, LogSeverity severity,
    // const std::string& message) to log messages. Note: Log a message with the specified layer and
    // severity.
    static void log(LogLayer layer, LogSeverity severity, const std::string& message);

   private:
    // Dev: The rule of five is private to prevent copying and moving of the singleton instance.

    Logger()                         = default;
    ~Logger()                        = default;
    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(const Logger&) = delete;

    static std::vector<std::unique_ptr<LoggerInterface>> m_loggers;
    static std::mutex                                    m_mutex;
};

}  // namespace nbody

#ifdef DEBUG
#define LOG_BASE_INFO(message) \
    nbody::Logger::log(nbody::LogLayer::BASE, nbody::LogSeverity::INFO, message)
#define LOG_BASE_WARNING(message) \
    nbody::Logger::log(nbody::LogLayer::BASE, nbody::LogSeverity::WARNING, message)
#define LOG_BASE_ERROR(message) \
    nbody::Logger::log(nbody::LogLayer::BASE, nbody::LogSeverity::ERROR, message)
#define LOG_BASE_FATAL(message) \
    nbody::Logger::log(nbody::LogLayer::BASE, nbody::LogSeverity::FATAL, message)

#define LOG_RENDERER_INFO(message) \
    nbody::Logger::log(nbody::LogLayer::RENDERER, nbody::LogSeverity::INFO, message)
#define LOG_RENDERER_WARNING(message) \
    nbody::Logger::log(nbody::LogLayer::RENDERER, nbody::LogSeverity::WARNING, message)
#define LOG_RENDERER_ERROR(message) \
    nbody::Logger::log(nbody::LogLayer::RENDERER, nbody::LogSeverity::ERROR, message)
#define LOG_RENDERER_FATAL(message) \
    nbody::Logger::log(nbody::LogLayer::RENDERER, nbody::LogSeverity::FATAL, message)

#define LOG_APP_INFO(message) \
    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, message)
#define LOG_APP_WARNING(message) \
    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::WARNING, message)
#define LOG_APP_ERROR(message) \
    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::ERROR, message)
#define LOG_APP_FATAL(message) \
    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::FATAL, message)
#else
#define LOG_BASE_INFO(message)
#define LOG_BASE_WARN(message)
#define LOG_BASE_ERROR(message)
#define LOG_BASE_FATAL(message)

#define LOG_RENDERER_INFO(message)
#define LOG_RENDERER_WARN(message)
#define LOG_RENDERER_ERROR(message)
#define LOG_RENDERER_FATAL(message)

#define LOG_APP_INFO(message)
#define LOG_APP_WARN(message)
#define LOG_APP_ERROR(message)
#define LOG_APP_FATAL(message)
#endif
