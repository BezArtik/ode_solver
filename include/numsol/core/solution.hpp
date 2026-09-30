#pragma once

#include <cstddef>
#include <vector>

#include "numsol/core/types.hpp"

namespace numsol {

struct solver_stats {
    std::size_t steps_ = 0;
    std::size_t rhs_evals_ = 0;
    time last_h_ = 0.0;
    bool success_ = true;
};

struct solution {
    std::vector<time> t_;
    std::vector<state> y_;
    solver_stats stats_;
};

}  // namespace numsol
