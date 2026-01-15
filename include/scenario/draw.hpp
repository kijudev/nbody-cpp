// ==============================================================================
// draw.hpp
// Drawing utilities for scenario UI overlays and simulation visualization.
// Provides helpers for rulers, body rendering, labels, and crosshairs.
// ==============================================================================

#pragma once

#include <span>
#include <string>
#include <utility>
#include <vector>

#include "base/type.hpp"
#include "gfx/box.hpp"
#include "gfx/camera.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

// ==============================================================================
// Drawing Functions for Scenario UI and Visualization
// ==============================================================================

// Draws a ruler in astronomical units (AU) at the bottom of the screen.
// The ruler length is chosen to fit a fraction of the screen width.
template <FloatT Float>
void draw_ruler_au(const gfx::Camera<Float>& camera, I32 ruler_padding,
                   I32 ruler_height, Float max_screen_fraction, Color color);

// Draws all bodies as filled circles of a single color.
template <FloatT Float>
void draw_bodies_monocolor(
    const gfx::Camera<Float> camera, Float scale_factor,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies,
    Color                                                   color);

// Draws a vertical list of text labels inside a box, with spacing and font
// size.
void draw_labels_vertical(const gfx::Box&                box,
                          const std::vector<std::string> labels, I32 font_size,
                          I32 gap, Color color);

// Draws a crosshair at the center of the camera viewport.
template <FloatT Float>
void draw_cross_center(const gfx::Camera<Float>& camera, I32 size,
                       I32 thickness, Color color);

// Draws pairs of labels (key-value) in two columns inside a box.
void draw_label_pairs(
    const gfx::Box&                                        box,
    const std::vector<std::pair<std::string, std::string>> label_pairs,
    I32 font_size, I32 gap, I32 offset, Color color_a, Color color_b);

}  // namespace nbody::scenario::impl
