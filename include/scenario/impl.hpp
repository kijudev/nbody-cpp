#pragma once

#include <optional>
#include <span>
#include <string>

#include "base/type.hpp"
#include "gfx/camera.hpp"
#include "math/vec.hpp"
#include "sim/type.hpp"

namespace nbody::scenario::impl {
using namespace nbody::base::type;

template <FloatT Float>
std::string format_time(Float factor, Float year, Float day, Float hour,
                        Float minute);

template <FloatT Float>
std::optional<USize> get_body_at_mouse_position_au(
    gfx::Camera<Float>                                      camera,
    std::span<const sim::BodyT<Float>, std::dynamic_extent> bodies);

template <FloatT Float>
struct SlingshotState {
    bool               is_active{false};
    math::Vec2T<Float> start_pos{0.0, 0.0};
    math::Vec2T<Float> current_pos{0.0, 0.0};
    Float              base_mass{1.0};
    Float              radius_scale{0.5};
    Float              velocity_scale{1.0};
};

template <FloatT Float>
void handle_slingshot_input(SlingshotState<Float>&    state,
                            const gfx::Camera<Float>& camera);

template <FloatT Float>
void draw_slingshot(const SlingshotState<Float>& state,
                    const gfx::Camera<Float>& camera, Float scale_factor);

template <FloatT Float>
sim::BodyT<Float> create_slingshot_body(const SlingshotState<Float>& state,
                                        const gfx::Camera<Float>&    camera);

}  // namespace nbody::scenario::impl
