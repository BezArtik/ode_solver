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
};

}  // namespace numsol
