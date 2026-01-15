// ==============================================================================
// kepler.hpp
// Analytical solution for the two-body problem using Keplerian orbital
// elements. Provides a simulation class for computing orbits, energies, and
// angular momentum.
// ==============================================================================

#pragma once

#include <span>
#include <vector>

#include "base/type.hpp"
#include "sim/const.hpp"
#include "sim/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
class Kepler : public SimInterface<Float> {
   public:
    // ==============================================================================
    // Type Aliases
    // ==============================================================================
    using Vec2 = math::Vec2T<Float>;  // 2D vector type
    using Body = BodyT<Float>;        // Body type

    // ==============================================================================
    // Orbital Elements
    // ==============================================================================

    // Stores the parameters describing the Keplerian orbit.
    struct OrbitalElements {
        Float semi_major_axis;  // a: Semi-major axis
        Float
            eccentricity;   // e: Eccentricity (0 = circle, 0 < e < 1 = ellipse)
        Float inclination;  // i: Inclination (always 0 for 2D)
        Float longitude_ascending;  // Omega: Longitude of ascending node
                                    // (always 0 for 2D)
        Float argument_periapsis;   // omega: Argument of periapsis
        Float mean_anomaly_epoch;   // M0: Mean anomaly at epoch
        Float orbital_period;       // T: Orbital period
        Float mean_motion;          // n: Mean motion (2*PI/T)
    };

    // ==============================================================================
    // Configuration Struct
    // ==============================================================================

    // Specifies initial bodies, gravitational constant, and epoch.
    struct Config {
        Body  body1{};               // Primary body
        Body  body2{};               // Secondary body
        Float g{sim::scale_toy::G};  // Gravitational constant
        Float epoch{0.0};            // Initial time
        bool  compute_elements_from_state{
            true};  // Compute orbital elements from initial state
    };

    // ==============================================================================
    // Public Interface
    // ==============================================================================

    // Construct a Kepler simulation with the given config.
    Kepler(const Config& config);

    // Advance the simulation by one timestep (dt).
    void step(Float dt) override;

    // Set the simulation time directly.
    void set_time(Float t);

    // Insert a new body into the simulation.
    void insert_body(Body&& body) override;

    // Get the current simulation time.
    [[nodiscard]] Float time() const;

    // Get a span of all bodies in the simulation (read-only).
    [[nodiscard]] std::span<const Body, std::dynamic_extent> bodies()
        const override;

    // Get the current orbital elements.
    [[nodiscard]] const OrbitalElements& orbital_elements() const;

    // Get system properties.
    [[nodiscard]] Float total_mass() const;
    [[nodiscard]] Float reduced_mass() const;
    [[nodiscard]] Float total_energy() const;
    [[nodiscard]] Float total_angular_momentum() const;

   private:
    // Compute orbital elements from the current state.
    void compute_orbital_elements_from_state();

    // Update body positions and velocities from orbital elements.
    void update_bodies_from_orbital_elements();

    // Solve Kepler's equation for mean anomaly and eccentricity.
    Float solve_keplers_equation(Float mean_anomaly, Float eccentricity) const;

    // Compute center of mass position and velocity.
    Vec2 compute_center_of_mass() const;
    Vec2 compute_center_of_mass_velocity() const;

    // Simulation state
    std::vector<Body> m_bodies;
    Float             m_g;
    Float             m_current_time;
    Float             m_epoch;
    OrbitalElements   m_orbital_elements;

    // System properties
    Float m_total_mass;
    Float m_reduced_mass;
    Vec2  m_com_position;
    Vec2  m_com_velocity;
    Float m_total_energy;
    Float m_angular_momentum;
};

}  // namespace nbody::sim
