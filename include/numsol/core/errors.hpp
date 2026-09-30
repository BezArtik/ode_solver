/**
 * @file errors.hpp
 * @brief Exception hierarchy for solver errors.
 *
 * All errors thrown by the library derive from @ref numsol::solver_error,
 * which in turn derives from @c std::runtime_error. This allows callers
 * to catch all library-specific errors with a single handler.
 */

#pragma once

#include <stdexcept>

namespace numsol {

/**
 * @brief Base class for all solver errors.
 */
struct solver_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * @brief Thrown when the problem definition is invalid.
 *
 * Examples: @c t_end < @c t0, empty initial state, non-positive step
 * size, unknown method name.
 */
struct invalid_problem_error : solver_error {
    using solver_error::solver_error;
};

/**
 * @brief Thrown when the maximum number of steps is exceeded.
 */
struct max_steps_exceeded_error : solver_error {
    using solver_error::solver_error;
};

/**
 * @brief Thrown when the step size falls below @c h_min.
 */
struct step_size_too_small_error : solver_error {
    using solver_error::solver_error;
};

/**
 * @brief Thrown when an unimplemented feature is requested.
 */
struct not_implemented_error : solver_error {
    using solver_error::solver_error;
};

}  // namespace numsol
