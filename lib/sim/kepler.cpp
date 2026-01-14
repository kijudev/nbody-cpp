#include "sim/kepler.hpp"

#include <cmath>

#include "base/assert.hpp"
#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

template <FloatT Float>
Kepler<Float>::Kepler(const Config& config)
    : m_g(config.g), m_current_time(config.epoch), m_epoch(config.epoch) {
    m_bodies.push_back(config.body1);
    m_bodies.push_back(config.body2);

    ASSERT(m_bodies.size() == 2, "Kepler solution requires exactly 2 bodies");
    ASSERT(m_bodies[0].mass > 0.0 && m_bodies[1].mass > 0.0, "Body masses must be positive");

    m_total_mass   = m_bodies[0].mass + m_bodies[1].mass;
    m_reduced_mass = (m_bodies[0].mass * m_bodies[1].mass) / m_total_mass;
    m_com_position = compute_center_of_mass();
    m_com_velocity = compute_center_of_mass_velocity();

    if (config.compute_elements_from_state) {
        compute_orbital_elements_from_state();
    }
}

template <FloatT Float>
void Kepler<Float>::step(Float dt) {
    m_current_time += dt;
    update_bodies_from_orbital_elements();
}

template <FloatT Float>
void Kepler<Float>::set_time(Float t) {
    m_current_time = t;
    update_bodies_from_orbital_elements();
}

template <FloatT Float>
void Kepler<Float>::insert_body(Body&& body) {
    (void)body;
    ASSERT(false, "Kepler solution only supports exactly 2 bodies - cannot insert additional bodies");
}

template <FloatT Float>
Float Kepler<Float>::time() const {
    return m_current_time;
}

template <FloatT Float>
std::span<const typename Kepler<Float>::Body, std::dynamic_extent> Kepler<Float>::bodies() const {
    return m_bodies;
}

template <FloatT Float>
const typename Kepler<Float>::OrbitalElements& Kepler<Float>::orbital_elements() const {
    return m_orbital_elements;
}

template <FloatT Float>
Float Kepler<Float>::total_mass() const {
    return m_total_mass;
}

template <FloatT Float>
Float Kepler<Float>::reduced_mass() const {
    return m_reduced_mass;
}

template <FloatT Float>
Float Kepler<Float>::total_energy() const {
    return m_total_energy;
}

template <FloatT Float>
Float Kepler<Float>::total_angular_momentum() const {
    return m_angular_momentum;
}

template <FloatT Float>
void Kepler<Float>::compute_orbital_elements_from_state() {
    const Vec2 r = m_bodies[1].pos.sub(m_bodies[0].pos);
    const Vec2 v = m_bodies[1].vel.sub(m_bodies[0].vel);

    const Float r_mag = r.length();
    const Float v_mag = v.length();

    ASSERT(r_mag > 0.0, "Bodies cannot be at the same position");

    // NOTE: Compute specific orbital energy (energy per unit reduced mass).
    // E = v²/2 - u/r; u = G*M
    const Float mu              = m_g * m_total_mass;
    const Float specific_energy = (v_mag * v_mag) / 2.0 - mu / r_mag;

    // NOTE: Compute angular momentum (scalar for 2D: L = r × v).
    // For 2D: L = r.x * v.y - r.y * v.x
    const Float angular_momentum = r.x * v.y - r.y * v.x;
    m_angular_momentum           = angular_momentum;

    // NOTE: Compute semi-major axis: a = -u / (2*E).
    ASSERT(specific_energy < 0.0, "Orbit must be bound (negative energy) for Kepler solution");
    m_orbital_elements.semi_major_axis = -mu / (2.0 * specific_energy);

    // NOTE: Compute eccentricity vector and magnitude.
    // e_vec = (v × L) / u - r / |r|
    const Float e_x                 = (v.y * angular_momentum) / mu - r.x / r_mag;
    const Float e_y                 = (-v.x * angular_momentum) / mu - r.y / r_mag;
    m_orbital_elements.eccentricity = std::sqrt(e_x * e_x + e_y * e_y);

    ASSERT(m_orbital_elements.eccentricity < 1.0,
           "Orbit must be elliptical (e < 1) for Kepler solution");

    // NOTE: Compute argument of periapsis (angle of eccentricity vector).
    m_orbital_elements.argument_periapsis = std::atan2(e_y, e_x);

    // NOTE: Compute orbital period: T = 2*PI * sqrt(a^3 / u).
    const Float a_cubed = m_orbital_elements.semi_major_axis * m_orbital_elements.semi_major_axis *
                          m_orbital_elements.semi_major_axis;
    m_orbital_elements.orbital_period = 2.0 * M_PI * std::sqrt(a_cubed / mu);

    // NOTE: Compute mean motion: n = 2*PI / T
    m_orbital_elements.mean_motion = 2.0 * M_PI / m_orbital_elements.orbital_period;

    // NOTE: Compute true anomaly at epoch (angle from periapsis).
    // cos(ν) = (e_vec · r) / (e * r)
    const Float cos_nu       = (e_x * r.x + e_y * r.y) / (m_orbital_elements.eccentricity * r_mag);
    const Float sin_nu       = (e_x * r.y - e_y * r.x) / (m_orbital_elements.eccentricity * r_mag);
    const Float true_anomaly = std::atan2(sin_nu, cos_nu);

    // NOTE: Compute eccentric anomaly from true anomaly.
    // tan(E/2) = sqrt((1-e)/(1+e)) * tan(ν/2)
    const Float e           = m_orbital_elements.eccentricity;
    const Float sqrt_factor = std::sqrt((1.0 - e) / (1.0 + e));
    const Float E           = 2.0 * std::atan(sqrt_factor * std::tan(true_anomaly / 2.0));

    // NOTE: Compute mean anomaly at epoch: M = E - e * sin(E).
    m_orbital_elements.mean_anomaly_epoch = E - e * std::sin(E);

    // NOTE: For a 2D orbit the inclination and longitude of ascending node are zero.
    m_orbital_elements.inclination         = 0.0;
    m_orbital_elements.longitude_ascending = 0.0;

    // NOTE: Compute total energy.
    m_total_energy = specific_energy * m_reduced_mass;
}

template <FloatT Float>
void Kepler<Float>::update_bodies_from_orbital_elements() {
    const Float dt = m_current_time - m_epoch;

    // NOTE: Compute mean anomaly at current time.
    const Float M = m_orbital_elements.mean_anomaly_epoch + m_orbital_elements.mean_motion * dt;

    // NOTE: Solve Kepler's equation for eccentric anomaly.
    const Float E = solve_keplers_equation(M, m_orbital_elements.eccentricity);

    // NOTE: Compute true anomaly from eccentric anomaly.
    // tan(ν/2) = sqrt((1+e)/(1-e)) * tan(E/2)
    const Float e           = m_orbital_elements.eccentricity;
    const Float sqrt_factor = std::sqrt((1.0 + e) / (1.0 - e));
    const Float nu          = 2.0 * std::atan(sqrt_factor * std::tan(E / 2.0));

    // NOTE: Compute distance from focus.
    const Float a = m_orbital_elements.semi_major_axis;
    const Float r = a * (1.0 - e * std::cos(E));

    // NOTE: Compute position in orbital plane (periapsis at x-axis).
    const Float x_orb = r * std::cos(nu);
    const Float y_orb = r * std::sin(nu);

    // NOTE: Rotate by argument of periapsis.
    const Float omega     = m_orbital_elements.argument_periapsis;
    const Float cos_omega = std::cos(omega);
    const Float sin_omega = std::sin(omega);

    const Float x_rot = x_orb * cos_omega - y_orb * sin_omega;
    const Float y_rot = x_orb * sin_omega + y_orb * cos_omega;

    // NOTE: Compute velocity in orbital plane.
    // v = sqrt(u/a) / sqrt(1 - e^2)
    const Float mu = m_g * m_total_mass;
    const Float h  = std::sqrt(mu * a * (1.0 - e * e));  // Angular momentum magnitude

    const Float vx_orb = -h * std::sin(nu) / r;
    const Float vy_orb = h * (e + std::cos(nu)) / r;

    // NOTE. Rotate velocity.
    const Float vx_rot = vx_orb * cos_omega - vy_orb * sin_omega;
    const Float vy_rot = vx_orb * sin_omega + vy_orb * cos_omega;

    // NOTE: Convert from relative coordinates to individual body coordinates using center of mass.
    const Float m1 = m_bodies[0].mass;
    const Float m2 = m_bodies[1].mass;

    // NOTE: Body 1 (relative to center of mass).
    m_bodies[0].pos.x = m_com_position.x - m2 / m_total_mass * x_rot;
    m_bodies[0].pos.y = m_com_position.y - m2 / m_total_mass * y_rot;
    m_bodies[0].vel.x = m_com_velocity.x - m2 / m_total_mass * vx_rot;
    m_bodies[0].vel.y = m_com_velocity.y - m2 / m_total_mass * vy_rot;

    // NOTE: Body 2 (relative to center of mass).
    m_bodies[1].pos.x = m_com_position.x + m1 / m_total_mass * x_rot;
    m_bodies[1].pos.y = m_com_position.y + m1 / m_total_mass * y_rot;
    m_bodies[1].vel.x = m_com_velocity.x + m1 / m_total_mass * vx_rot;
    m_bodies[1].vel.y = m_com_velocity.y + m1 / m_total_mass * vy_rot;

    // NOTE: Compute accelerations; for completeness sake; not used in the analytical solution.
    const Vec2  r12       = m_bodies[1].pos.sub(m_bodies[0].pos);
    const Float r12_mag   = r12.length();
    const Float r12_cubed = r12_mag * r12_mag * r12_mag;

    m_bodies[0].acc.x = m_g * m2 * r12.x / r12_cubed;
    m_bodies[0].acc.y = m_g * m2 * r12.y / r12_cubed;
    m_bodies[1].acc.x = -m_g * m1 * r12.x / r12_cubed;
    m_bodies[1].acc.y = -m_g * m1 * r12.y / r12_cubed;
}

template <FloatT Float>
Float Kepler<Float>::solve_keplers_equation(Float M, Float e) const {
    // EXPLANATION: Solve M = E - e * sin(E) using Newton-Raphson iteration.

    // NOTE: Normalize mean anomaly to [0, 2*PI]
    Float M_norm = std::fmod(M, 2.0 * M_PI);
    if (M_norm < 0.0) {
        M_norm += 2.0 * M_PI;
    }

    // NOTE: Initial guess for eccentric anomaly.
    Float E = M_norm;
    if (e > 0.8) {
        E = M_PI;  // WHY: Better initial guess for high eccentricity.
    }

    // NOTE: Newton-Raphson iteration.
    const USize max_iterations = 100;
    const Float tolerance      = static_cast<Float>(1e-10);

    for (USize i = 0; i < max_iterations; ++i) {
        const Float f  = E - e * std::sin(E) - M_norm;
        const Float df = 1.0 - e * std::cos(E);

        const Float delta = f / df;
        E -= delta;

        if (std::abs(delta) < tolerance) {
            break;
        }
    }

    return E;
}

template <FloatT Float>
typename Kepler<Float>::Vec2 Kepler<Float>::compute_center_of_mass() const {
    const Float m1 = m_bodies[0].mass;
    const Float m2 = m_bodies[1].mass;

    Vec2 com;
    com.x = (m1 * m_bodies[0].pos.x + m2 * m_bodies[1].pos.x) / m_total_mass;
    com.y = (m1 * m_bodies[0].pos.y + m2 * m_bodies[1].pos.y) / m_total_mass;

    return com;
}

template <FloatT Float>
typename Kepler<Float>::Vec2 Kepler<Float>::compute_center_of_mass_velocity() const {
    const Float m1 = m_bodies[0].mass;
    const Float m2 = m_bodies[1].mass;

    Vec2 com_vel;
    com_vel.x = (m1 * m_bodies[0].vel.x + m2 * m_bodies[1].vel.x) / m_total_mass;
    com_vel.y = (m1 * m_bodies[0].vel.y + m2 * m_bodies[1].vel.y) / m_total_mass;

    return com_vel;
}

template class Kepler<F32>;
template class Kepler<F64>;

}  // namespace nbody::sim
