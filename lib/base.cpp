#include "base.hpp"

namespace nbody {
void Logger::log(LoggerLayer layer, LoggerSeverity severity, const std::string& message) {

}

U16 Logger::impl_get_layers_mask(const std::vector<LoggerLayer>& layers) {
    U16 mask = 0;
    for (LoggerLayer layer : layers) {
        mask |= static_cast<U16>(layer);
    }
    return mask;
}

U16 Logger::impl_get_severities_mask(const std::vector<LoggerSeverity>& severities) {
    U16 mask = 0;
    for (LoggerSeverity severity : severities) {
        mask |= static_cast<U16>(severity);
    }
    return mask;
}
}  // namespace nbody
