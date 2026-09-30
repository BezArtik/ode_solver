/**
 * @file rk3.hpp
 * @brief Third-order Runge-Kutta method (Kutta).
 */

#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

/**
 * @brief Butcher tableau of Kutta's third-order method.
 */
struct rk3_tableau {
    /// @name Butcher tableau coefficients
    /// Coefficients follow the convention described in
    /// @ref numsol::explicit_rk.
    /// @{
    static constexpr std::array<scalar, 3> c{0.0, 0.5, 1.0};
    static constexpr std::array<std::array<scalar, 3>, 3> a{{
        {0.0, 0.0, 0.0},
        {0.5, 0.0, 0.0},
        {-1.0, 2.0, 0.0},
    }};
    static constexpr std::array<scalar, 3> b{
        1.0 / 6.0,
        2.0 / 3.0,
        1.0 / 6.0,
    };
    static constexpr std::int32_t order = 3;
    /// @}
};

/**
 * @brief Classical Kutta's third-order method.
 */
using rk3 = explicit_rk<rk3_tableau>;

}  // namespace numsol
