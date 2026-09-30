#pragma once

#include <concepts>

#include "numsol/core/types.hpp"

namespace numsol {

template <typename F>
concept rhs = requires(F&& f, time t, state_view y) {
    { f(t, y) } -> std::convertible_to<state>;
};

template <rhs F>
struct problem {
    F rhs_;
    state y0_;
    time t0_ = 0.0;
    time t_end_ = 0.0;
};

}  // namespace numsol
