#pragma once

#include "base/type.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
void integrate_body_euler(BodyT<Float>& body, Float dt);

template <FloatT Float>
void integrate_body_verlet(BodyT<Float>& body, Float dt);
}  // namespace nbody::sim
