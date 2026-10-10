/**
 * @file problem.hpp
 * @brief Initial value problem definition.
 */

#pragma once

#include "numsol/core/types.hpp"

#include <concepts>

namespace numsol {

/**
 * @concept rhs
 * @brief Requirements for a right-hand side function.
 *
 * A valid RHS is a callable object that takes a time value and a
 * read-only state view, and returns a new state vector:
 *
 * @code
 * state f(time t, state_view y);
 * @endcode
 */
template <typename F>
concept rhs = requires(F&& f, time t, state_view y) {
    { f(t, y) } -> std::convertible_to<state>;
};

/**
 * @brief Initial value problem: @c y' = f(t, y), @c y(t0) = y0.
 *
 * @tparam F Right-hand side type satisfying @ref rhs.
 */
template <rhs F>
struct problem {
    /// Right-hand side function.
    F rhs_;

    /// Initial state at @ref t0_.
    state y0_;

    /// Initial time.
    time t0_ = 0.0;

    /// Final time.
    time t_end_ = 0.0;
};

}  // namespace numsol
