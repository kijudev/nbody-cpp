#pragma once

#include "base/type.hpp"

namespace nbody::sim {
using namespace nbody::base::type;

static constexpr F64 G_IRL = 6.67430e-11;
static constexpr F64 G_TOY = 1.0;

static constexpr F64 SOFTENING_IRL = 1e-6;
static constexpr F64 SOFTENING_TOY = 0.1;

namespace scale_au {
// UNIT: 149 597 871 000m.
static constexpr F64 UNIT_AU = 1.0;

// UNIT: 1.9891 * 10^30kg.
static constexpr F64 MASS_SOL     = 1.0;
static constexpr F64 MASS_EARTH   = 3.003e-6;
static constexpr F64 MASS_MARS    = 3.227e-7;
static constexpr F64 MASS_JUPITER = 9.543e-4;
static constexpr F64 MASS_PLUTO   = 6.58e-9;
static constexpr F64 MASS_CERES   = 4.72e-10;
static constexpr F64 MASS_HYGIEA  = 4.375e-11;

// UNIT: 86,400s.
static constexpr F64 TIME_DAY     = 1.0;
static constexpr F64 TIME_YEAR    = TIME_DAY * 365.25;
static constexpr F64 TIME_MILENIA = TIME_YEAR * 1000.0;
static constexpr F64 TIME_HOUR    = TIME_DAY / 24.0;
static constexpr F64 TIME_MINUTE  = TIME_HOUR / 60.0;
static constexpr F64 TIME_SECOND  = TIME_MINUTE / 60.0;

// UNIT: AU, Solar Mass, Solar Day.
static constexpr F64 CONST_G = 0.00029591220828;
;
}  // namespace scale_au
}  // namespace nbody::sim
