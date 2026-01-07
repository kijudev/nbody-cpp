#pragma once

#include <string>

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

struct Window {
    I32         width{800};
    I32         height{600};
    std::string title{};

    void init() const;

    // TODO: Implement.
    void handle_resize();
};
}  // namespace nbody::gfx
