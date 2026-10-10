/**
 * @file solution.hpp
 * @brief Numerical solution and integration statistics.
 */

#pragma once

#include "numsol/core/stats.hpp"
#include "numsol/core/types.hpp"

#include <vector>

namespace numsol {

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

    /// Coarse solutions (full-step) for double-step methods.
    std::vector<state> y_coarse_;

    /// Local error estimates per step (double-step methods).
    std::vector<scalar> lee_;

    /// Step size used at each step.
    std::vector<time> h_;

    /// Cumulative halvings after each step (C1).
    std::vector<std::size_t> c1_;

    /// Cumulative doublings after each step (C2).
    std::vector<std::size_t> c2_;
};

}  // namespace numsol
