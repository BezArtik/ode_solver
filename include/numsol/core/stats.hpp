/**
 * @file stats.hpp
 * @brief Integration statistics.
 */

#pragma once

#include "numsol/core/types.hpp"

#include <cstddef>

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

    /// Number of times the step size was halved (C1).
    std::size_t halvings_ = 0;

    /// Number of times the step size was doubled (C2).
    std::size_t doublings_ = 0;

    /// Maximum local error estimate.
    scalar max_lee_ = 0.0;

    /// Time at which @ref max_lee_ was observed.
    time max_lee_at_ = 0.0;

    /// Maximum step size used.
    time max_h_ = 0.0;

    /// Time at which @ref max_h_ was used.
    time max_h_at_ = 0.0;

    /// Minimum step size used.
    time min_h_ = 0.0;

    /// Time at which @ref min_h_ was used.
    time min_h_at_ = 0.0;
};

}  // namespace numsol
