// ==============================================================================
// const.hpp
// Physical constants and preset scales for n-body simulations.
// Defines toy and astronomical unit (AU) scales for distance, mass, time, and
// simulation parameters like G and softening.
// ==============================================================================

#pragma once

#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

// ==============================================================================
// Toy Scale Constants
// ==============================================================================
// Simple unit scale for toy simulations (distance, mass, time all normalized).
// Useful for testing and debugging.
namespace scale_toy {
static constexpr F64 DISTANCE = 1.0;  // Unit distance
static constexpr F64 MASS     = 1.0;  // Unit mass
static constexpr F64 TIME     = 1.0;  // Unit time

static constexpr F64 G =
    0.00029591220828;  // Gravitational constant (toy units)
static constexpr F64 SOFTENING =
    DISTANCE * 5.0e-5;  // Softening parameter (toy units)
}  // namespace scale_toy

// ==============================================================================
// Astronomical Unit (AU) Scale Constants
// ==============================================================================
// Realistic scale for solar system and galaxy simulations.
// All values are normalized to AU, Solar Mass, and Solar Day.
namespace scale_au {
static constexpr F64 DISTANCE_AU = 1.0;  // 1 AU = 149,597,871,000 meters

static constexpr F64 MASS_SOL = 1.0;  // Solar mass (1.9891e30 kg)
static constexpr F64 MASS_EARTH =
    3.003e-6;  // Earth mass (fraction of solar mass)
static constexpr F64 MASS_MARS    = 3.227e-7;   // Mars mass
static constexpr F64 MASS_JUPITER = 9.543e-4;   // Jupiter mass
static constexpr F64 MASS_PLUTO   = 6.58e-9;    // Pluto mass
static constexpr F64 MASS_CERES   = 4.72e-10;   // Ceres mass
static constexpr F64 MASS_HYGIEA  = 4.375e-11;  // Hygiea mass

static constexpr F64 TIME_DAY     = 1.0;  // 1 day = 86,400 seconds
static constexpr F64 TIME_YEAR    = TIME_DAY * 365.25;   // 1 year
static constexpr F64 TIME_MILENIA = TIME_YEAR * 1000.0;  // 1 milenia
static constexpr F64 TIME_HOUR    = TIME_DAY / 24.0;     // 1 hour
static constexpr F64 TIME_MINUTE  = TIME_HOUR / 60.0;    // 1 minute
static constexpr F64 TIME_SECOND  = TIME_MINUTE / 60.0;  // 1 second

static constexpr F64 G =
    0.00029591220828;  // Gravitational constant (AU, Solar Mass, Solar Day)

static constexpr F64 SOFTENING =
    DISTANCE_AU * 5.0e-5;  // Softening parameter (~Earth's radius)
}  // namespace scale_au

}  // namespace nbody::sim
