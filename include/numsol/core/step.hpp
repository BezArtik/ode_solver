/**
 * @file step.hpp
 * @brief Result of a single integration step.
 */

#pragma once

#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Outcome of one step of a numerical method.
 *
 * Returned by @c step() of every integration method. Contains the
 * state and time after the step, along with optional error control
 * information used by adaptive methods.
 *
 * Fixed-step methods leave @ref error_estimate_ at zero,
 * @ref suggested_h_ at zero, and @ref accepted_ at @c true.
 */
struct step_result {
    /// State at @ref t_next_.
    state y_next_;

    /// Time after the step.
    time t_next_ = 0.0;

    /// Estimate of the local error, normalized by the tolerance.
    ///
    /// A value less than or equal to 1 means the step is accepted.
    /// Fixed-step methods leave this at zero.
    scalar error_estimate_ = 0.0;

    /// Suggested step size for the next step.
    ///
    /// Ignored by fixed-step methods; used by adaptive ones.
    time suggested_h_ = 0.0;

    /// Whether the step is accepted.
    ///
    /// Fixed-step methods always set this to @c true.
    bool accepted_ = true;

    /// Coarse solution obtained with the full step.
    ///
    /// Populated only by double-step methods such as
    /// @ref rk4_adaptive. Empty otherwise.
    state y_coarse_;

    /// Local error estimate @c |v_i - v_2i| for double-step
    /// methods. Zero otherwise.
    scalar lee_ = 0.0;

    /// True if the step size was halved after this step.
    bool halved_ = false;

    /// True if the step size was doubled after this step.
    bool doubled_ = false;
};

}  // namespace numsol
