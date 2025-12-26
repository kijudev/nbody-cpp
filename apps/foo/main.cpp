// This file exists only for testing purposes.

#include "base/base.hpp"
#include "base/base_assert.hpp"

int main() {
    nbody::Logger::init();

    LOG_APP_INFO("asdasdasd");

    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, "Application started");

    ASSERT(1 > 2, "1 is not greater than 2");

    return 0;
}
