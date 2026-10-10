/**
 * @file types.hpp
 * @brief Fundamental scalar and state types.
 *
 * This header defines the basic type aliases used throughout the
 * library.
 *
 */

#pragma once

#include "numsol/utils/small_vector.hpp"

#include <span>

/**
 * @namespace numsol
 * @brief Root namespace of the library.
 *
 * Contains the reusable ODE solver core (`numsol::core`,
 * `numsol::methods`) and the application layer (`numsol::app`).
 */
namespace numsol {

/**
 * @typedef scalar
 * @brief Floating-point type used for all numerical computations.
 */
using scalar = double;

/**
 * @typedef time
 * @brief Type used for time and time intervals.
 *
 * Alias for @ref scalar. Kept separate to make function signatures
 * self-documenting (a parameter of type `time` is clearly a time
 * value, not a generic scalar).
 */
using time = scalar;

/**
 * @typedef state
 * @brief Owning container for a system state vector.
 *
 * A `state` holds the values of all components of the ODE system at
 * a single time point. The length of the vector equals the dimension
 * of the system.
 */
using state = small_vector<scalar, 8>;

/**
 * @typedef state_view
 * @brief Non-owning read-only view of a state vector.
 *
 * Used to pass state data to functions (e.g. the right-hand side)
 * without copying or transferring ownership. Compatible with any
 * contiguous range of `scalar`.
 */
using state_view = std::span<const scalar>;

}  // namespace numsol
