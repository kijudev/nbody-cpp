#include "base.hpp"

int main() {
    nbody::Logger::init();

    nbody::Logger::log(nbody::LogLayer::APP, nbody::LogSeverity::INFO, "Application started");

    return 0;
}
