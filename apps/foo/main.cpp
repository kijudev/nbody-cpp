// NOTE: This file exists only for testing purposes.

#include "log.hpp"

int main() {
    nbody::Logger::init();

    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, "Application started");

    LOG_APP_INFO("Application started");
    LOG_APP_WARNING("Application started");
    LOG_APP_ERROR("Application started");
    LOG_APP_FATAL("Application started");

    return 0;
}
