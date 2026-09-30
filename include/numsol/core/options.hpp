#pragma once

#include <cstddef>

#include "numsol/core/types.hpp"

namespace numsol {

struct solver_options {
    time h0_ = 1e-3;
    time h_min_ = 1e-12;
    time h_max_ = 1.0;
    std::size_t max_steps_ = 1'000'000;
    std::size_t save_every_ = 1;
};

}  // namespace numsol
