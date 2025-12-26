// This file exists only for testing purposes.

#include "base/base.hpp"

int main() {
    nbody::Logger::init();

    LOG_APP_INFO("asdasdasd");

    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, "Application started");

    return 0;
}
