/**
 * @file solution.hpp
 * @brief Numerical solution and integration statistics.
 */

#pragma once

#include <cstddef>
#include <vector>

#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Statistics collected during integration.
 */
struct solver_stats {
    /// Number of accepted steps.
    std::size_t steps_ = 0;

    /// Total number of right-hand side evaluations.
    std::size_t rhs_evals_ = 0;

    /// Step size of the last step.
    time last_h_ = 0.0;

    /// True if integration reached @c t_end.
    bool success_ = true;
};

/**
 * @brief Numerical solution of an initial value problem.
 *
 * Stores the trajectory in structure-of-arrays form: @ref t_ holds
 * time points and @ref y_ holds the corresponding state vectors.
 * The two arrays have the same length.
 */
struct solution {
    /// Time points.
    std::vector<time> t_;

    /// State vectors, one per entry in @ref t_.
    std::vector<state> y_;

    /// Integration statistics.
    solver_stats stats_;
};

}  // namespace numsol
