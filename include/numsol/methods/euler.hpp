#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

struct euler_tableau {
    static constexpr std::array<scalar, 1> c{0.0};
    static constexpr std::array<std::array<scalar, 1>, 1> a{{0.0}};
    static constexpr std::array<scalar, 1> b{1.0};
    static constexpr std::int32_t order = 1;
};

using euler = explicit_rk<euler_tableau>;

}  // namespace numsol
