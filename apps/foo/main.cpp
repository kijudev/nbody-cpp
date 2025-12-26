// NOTE: This file exists only for testing purposes.

// --- Header Files ---
#include "base/base.hpp"

// --- Source Files ---
#include "base/base.cpp"

int main() {
    nbody::Logger::init();

    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, "Application started");

    return 0;
}
