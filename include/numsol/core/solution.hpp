/**
 * @file solution.hpp
 * @brief Numerical solution and integration statistics.
 */

#pragma once

#include <vector>

#include "numsol/core/stats.hpp"
#include "numsol/core/types.hpp"

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
};

}  // namespace numsol
