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
 * state and time after the step.
 */
struct step_result {
    /// State at @ref t_next_.
    state y_next_;

    /// Time after the step.
    time t_next_ = 0.0;
};

}  // namespace numsol
