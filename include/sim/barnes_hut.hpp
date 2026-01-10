#pragma once

#include <array>
#include <span>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/integrator.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class BarnesHut : public SimInterface<Float> {
   public:
    // --- General Typedefs ---
    using Vec2      = math::Vec2T<Float>;
    using Body      = BodyT<Float>;
    using QuadIndex = U8;
    using NodeIndex = USize;

    // --- Quad Tree ---
    static constexpr NodeIndex NODE_INDEX_EMPTY = 0;

    struct Quad {
        Vec2  center;
        Float size;

        static Quad make_containing_bodies(std::span<Body, std::dynamic_extent> bodies);

        Quad                into_quad(QuadIndex quad_index) const;
        std::array<Quad, 4> into_quads() const;
        Quad                quad_from_pos(Vec2 pos) const;
    };

    struct Node {
        NodeIndex children_index;
        NodeIndex next_index;
        Quad      quad;
        Vec2      pos;
        Float     mass;

        static Node make_empty(NodeIndex next_index, Quad quad);

        bool is_empty() const;
        bool is_inernal() const;
        bool is_leaf() const;
    };

    struct QuadTree {
        std::vector<Node>      nodes;
        std::vector<NodeIndex> parent_indices;

        void      clear(Quad quad);
        NodeIndex subdivide(NodeIndex node_index);
        void      insert(Vec2 pos, Float mass);
        void      propagate_up_pos_mass();
        Vec2      propagate_down_acc(Vec2 pos, Float g, Float softening, Float theta) const;
    };

    // --- Config ---
    struct Config {
        std::vector<Body>       bodies{};
        Float                   g{sim::scale_toy::G};
        Float                   softening{sim::scale_toy::SOFTENING};
        Float                   theta{0.5};
        IntegrateBodyFnT<Float> integrate_fn{integrate_body_euler<Float>};
    };

    // --- Public Interface ---
    BarnesHut(const Config& config);

    void step(Float dt);
    void insert(Body&& body);

    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies() const;
    [[nodiscard]] std::vector<Quad>                          quads() const;

   private:
    std::vector<Body>       m_bodies;
    QuadTree                m_quad_tree;
    Float                   m_g;
    Float                   m_softening;
    Float                   m_theta;
    IntegrateBodyFnT<Float> m_integrate_fn;
};
}  // namespace nbody::sim
