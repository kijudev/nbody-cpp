#pragma once

#include <string>

#include "base/type.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

struct Window {
    static constexpr I32 DEAFULT_WITDH = 800;
    static constexpr I32 DEAFULT_HEIGHT = 800;

    I32         width{DEAFULT_WITDH};
    I32         height{DEAFULT_HEIGHT};
    std::string title{};

    void init() const;
    void handle_resize();
};
}  // namespace nbody::gfx
