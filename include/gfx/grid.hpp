#include "base/type.hpp"
#include "gfx/box.hpp"

namespace nbody::gfx {
using namespace nbody::base::type;

struct Grid {
    I32 width{0};
    I32 height{0};
    I32 cols{1};
    I32 rows{1};

    // NOTE: Columns and rows are indexed from 0.
    Box span(I32 col_a, I32 row_a, I32 col_b, I32 row_b) const;

    I32 col_size() const;
    I32 row_size() const;
};
}  // namespace nbody::gfx
