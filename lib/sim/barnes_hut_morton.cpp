#include <span>
#include <utility>

#include "base/type.hpp"
#include "sim/barnes_hut_morton.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float, MortonCodeT MortonCode>
BarnesHutMorton<Float, MortonCode>::BarnesHutMorton(const Config& config)
    : m_integrate(config.integrate_fn),
      m_g(config.g),
      m_softening(config.softening),
      m_theta(config.theta),
      m_bodies(std::move(config.bodies)),
      m_bodies_lookup(m_bodies.size(), false) {}

template <FloatT Float, MortonCodeT MortonCode>
std::span<const typename BarnesHutMorton<Float, MortonCode>::Body, std::dynamic_extent>
BarnesHutMorton<Float, MortonCode>::bodies() const {
    return m_bodies;
}

template <FloatT Float, MortonCodeT MortonCode>
void BarnesHutMorton<Float, MortonCode>::insert_body(Body&& body) {
    if (m_dead_bodies_count == 0) {
        m_bodies.emplace_back(body);
        m_bodies_lookup.emplace_back(false);
        return;
    }

    --m_dead_bodies_count;
    USize dead_body_next = ;
}

template class BarnesHutMorton<F32, U32>;
template class BarnesHutMorton<F32, U64>;
template class BarnesHutMorton<F64, U32>;
template class BarnesHutMorton<F64, U64>;
}  // namespace nbody::sim
