#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

struct rk2_tableau {
    static constexpr std::array<scalar, 2> c{0.0, 1.0};
    static constexpr std::array<std::array<scalar, 2>, 2> a{{
        {0.0, 0.0},
        {1.0, 0.0},
    }};
    static constexpr std::array<scalar, 2> b{0.5, 0.5};
    static constexpr std::int32_t order = 2;
};

using rk2 = explicit_rk<rk2_tableau>;

}  // namespace numsol
