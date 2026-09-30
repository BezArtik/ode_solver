/**
 * @file rk2.hpp
 * @brief Second-order Runge-Kutta method (Heun).
 */

#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

/**
 * @brief Butcher tableau of Heun's method.
 */
struct rk2_tableau {
    /// @name Butcher tableau coefficients
    /// Coefficients follow the convention described in
    /// @ref numsol::explicit_rk.
    /// @{
    static constexpr std::array<scalar, 2> c{0.0, 1.0};
    static constexpr std::array<std::array<scalar, 2>, 2> a{{
        {0.0, 0.0},
        {1.0, 0.0},
    }};
    static constexpr std::array<scalar, 2> b{0.5, 0.5};
    static constexpr std::int32_t order = 2;
    /// @}
};

/**
 * @brief Heun's method (improved Euler).
 */
using rk2 = explicit_rk<rk2_tableau>;

}  // namespace numsol
