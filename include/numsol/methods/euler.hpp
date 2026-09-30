/**
 * @file euler.hpp
 * @brief Explicit Euler method.
 */

#pragma once

#include <array>

#include "numsol/methods/explicit_rk.hpp"

namespace numsol {

/**
 * @brief Butcher tableau of the explicit Euler method.
 */
struct euler_tableau {
    /// @name Butcher tableau coefficients
    /// Coefficients follow the convention described in
    /// @ref numsol::explicit_rk.
    /// @{
    static constexpr std::array<scalar, 1> c{0.0};
    static constexpr std::array<std::array<scalar, 1>, 1> a{{0.0}};
    static constexpr std::array<scalar, 1> b{1.0};
    static constexpr std::int32_t order = 1;
    /// @}
};

/**
 * @brief Explicit Euler method.
 */
using euler = explicit_rk<euler_tableau>;

}  // namespace numsol
