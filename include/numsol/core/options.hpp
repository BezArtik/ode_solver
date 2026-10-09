/**
 * @file options.hpp
 * @brief Solver configuration parameters.
 */

#pragma once

#include <cstddef>

#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Numerical parameters controlling the integration process.
 *
 * All fields have sensible defaults; callers override only what they
 * need.
 */
struct solver_options {
    /// Initial step size.
    time h0_ = 1e-3;

    /// Minimum allowed step size.
    time h_min_ = 1e-12;

    /// Maximum allowed step size.
    time h_max_ = 1.0;

    /// Maximum number of integration steps before aborting.
    std::size_t max_steps_ = 1'000'000;

    /// Enable adaptive step-size control.
    ///
    /// Has no effect with fixed-step methods.
    bool adaptive_ = false;

    /// Relative tolerance for adaptive step-size control.
    ///
    /// Used together with @ref atol_ to normalize the local error.
    scalar rtol_ = 1e-6;

    /// Absolute tolerance for adaptive step-size control.
    ///
    /// Used together with @ref rtol_ to normalize the local error.
    scalar atol_ = 1e-9;

    /// Safety factor applied when suggesting a new step size.
    ///
    /// Must be in @c (0, 1). Smaller values make the controller
    /// more conservative.
    scalar safety_ = 0.9;

    /// Threshold for the raw local error estimate used by
    /// double-step methods such as @ref rk4_adaptive.
    scalar eps_ = 1e-6;
};

}  // namespace numsol
