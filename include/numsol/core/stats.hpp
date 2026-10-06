/**
 * @file stats.hpp
 * @brief Integration statistics.
 */

#pragma once

#include <cstddef>

#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Statistics collected during integration.
 */
struct solver_stats {
    /// Number of accepted steps.
    std::size_t steps_ = 0;

    /// Number of rejected steps.
    ///
    /// Always zero for fixed-step methods.
    std::size_t rejected_ = 0;

    /// Total number of right-hand side evaluations.
    std::size_t rhs_evals_ = 0;

    /// Step size of the last accepted step.
    time last_h_ = 0.0;

    /// True if integration reached @c t_end.
    bool success_ = true;
};

}  // namespace numsol
