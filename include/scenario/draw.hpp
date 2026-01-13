#pragma once

#include <span>
#include <vector>

#include "base/type.hpp"
#include "gfx/box.hpp"
#include "gfx/camera.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

// TODO: Fix. Does not draw to scale.
template <FloatT Float>
void draw_ruler_au(const gfx::Camera<Float>& camera, I32 ruler_padding,
                   I32 ruler_height, Float max_screen_fraction);

template <FloatT Float>
void draw_bodies_monocolor(
    const gfx::Camera<Float> camera, Float scale_factor,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies,
    Color                                                   color);

void draw_labels_vertical(const gfx::Box&                box,
                          const std::vector<std::string> labels, I32 font_size,
                          I32 gap, Color color);

template <FloatT Float>
void draw_cross_center(const gfx::Camera<Float>& camera, I32 size,
                       I32 thickness, Color color);
}  // namespace nbody::scenario::impl
