#include "base/base.hpp"
#include "gfx/gfx.hpp"

int main() {
    nbody::Logger::init();
    nbody::TriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        LOG_APP_FATAL(e.what());
    }

    return 0;
}
