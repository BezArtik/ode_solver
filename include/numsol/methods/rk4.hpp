/**
 * @file rk4.hpp
 * @brief Classical fourth-order Runge-Kutta method.
 */

#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

/**
 * @brief Butcher tableau of the classical RK4 method.
 */
struct rk4_tableau {
    /// @name Butcher tableau coefficients
    /// Coefficients follow the convention described in
    /// @ref numsol::explicit_rk.
    /// @{
    static constexpr std::array<scalar, 4> c{0.0, 0.5, 0.5, 1.0};
    static constexpr std::array<std::array<scalar, 4>, 4> a{{
        {0.0, 0.0, 0.0, 0.0},
        {0.5, 0.0, 0.0, 0.0},
        {0.0, 0.5, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0},
    }};
    static constexpr std::array<scalar, 4> b{
        1.0 / 6.0,
        1.0 / 3.0,
        1.0 / 3.0,
        1.0 / 6.0,
    };
    static constexpr std::int32_t order = 4;
    /// @}
};

/**
 * @brief Classical fourth-order Runge-Kutta method.
 */
using rk4 = explicit_rk<rk4_tableau>;

}  // namespace numsol
