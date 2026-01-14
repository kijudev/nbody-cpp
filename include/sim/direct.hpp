#pragma once

#include <span>
#include <vector>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class Direct : public SimInterface<Float> {
   public:
    // --- General Typedefs ---
    using Vec2 = math::Vec2T<Float>;
    using Body = BodyT<Float>;

    // --- Config ---
    struct Config {
        std::vector<Body>       bodies{};
        Float                   g{sim::scale_toy::G};
        Float                   softening{sim::scale_toy::SOFTENING};
        bool                    parallel{false};
        IntegrateBodyFnT<Float> integrate_fn{integrate_body_euler<Float>};
    };

    // --- Public Interface ---
    Direct(const Config& config);

    void step(Float dt) override;
    void insert_body(Body&& body) override;

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const override;

   private:
    void impl_compute_acc();
    void impl_compute_acc_par();
    void impl_compute_acc_seq();

    std::vector<Body>       m_bodies;
    Float                   m_g;
    Float                   m_softening;
    bool                    m_parallel;
    IntegrateBodyFnT<Float> m_integrate_fn;
};
}  // namespace nbody::sim
