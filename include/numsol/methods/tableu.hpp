/**
 * @file tableau.hpp
 * @brief Butcher tableaus for explicit Runge-Kutta methods.
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Butcher tableau of the explicit Euler method.
 */
struct euler_tableau {
    static constexpr std::size_t stages = 1;

    static constexpr std::array<scalar, 1> c{0.0};
    static constexpr std::array<std::array<scalar, 1>, 1> a{{{0.0}}};
    static constexpr std::array<scalar, 1> b{1.0};

    static constexpr std::int32_t order = 1;
};

/**
 * @brief Butcher tableau of Heun's method.
 */
struct rk2_tableau {
    static constexpr std::size_t stages = 2;

    static constexpr std::array<scalar, 2> c{0.0, 1.0};
    static constexpr std::array<std::array<scalar, 2>, 2> a{{{0.0, 0.0}, {1.0, 0.0}}};
    static constexpr std::array<scalar, 2> b{0.5, 0.5};

    static constexpr std::int32_t order = 2;
};

/**
 * @brief Butcher tableau of Kutta's third-order method.
 */
struct rk3_tableau {
    static constexpr std::size_t stages = 3;

    static constexpr std::array<scalar, 3> c{0.0, 0.5, 1.0};
    static constexpr std::array<std::array<scalar, 3>, 3> a{{{0.0, 0.0, 0.0}, {0.5, 0.0, 0.0}, {-1.0, 2.0, 0.0}}};
    static constexpr std::array<scalar, 3> b{1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0};

    static constexpr std::int32_t order = 3;
};

/**
 * @brief Butcher tableau of the classical RK4 method.
 */
struct rk4_tableau {
    static constexpr std::size_t stages = 4;

    static constexpr std::array<scalar, 4> c{0.0, 0.5, 0.5, 1.0};
    static constexpr std::array<std::array<scalar, 4>, 4> a{
        {{0.0, 0.0, 0.0, 0.0}, {0.5, 0.0, 0.0, 0.0}, {0.0, 0.5, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}}};
    static constexpr std::array<scalar, 4> b{1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0};

    static constexpr std::int32_t order = 4;
};

/**
 * @brief Butcher tableau of the Dormand-Prince 5(4) method.
 *
 * Provides both fifth-order weights (@ref b) and an embedded
 * fourth-order estimate (@ref b_hat) for adaptive step control.
 */
struct dopri5_tableau {
    static constexpr std::size_t stages = 7;

    static constexpr std::int32_t order = 5;
    static constexpr std::int32_t order_low = 4;

    static constexpr std::array<scalar, 7> c{0.0, 1.0 / 5.0, 3.0 / 10.0, 4.0 / 5.0, 8.0 / 9.0, 1.0, 1.0};

    static constexpr std::array<std::array<scalar, 7>, 7> a{{
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {1.0 / 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {3.0 / 40.0, 9.0 / 40.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {44.0 / 45.0, -56.0 / 15.0, 32.0 / 9.0, 0.0, 0.0, 0.0, 0.0},
        {19372.0 / 6561.0, -25360.0 / 2187.0, 64448.0 / 6561.0, -212.0 / 729.0, 0.0, 0.0, 0.0},
        {9017.0 / 3168.0, -355.0 / 33.0, 46732.0 / 5247.0, 49.0 / 176.0, -5103.0 / 18656.0, 0.0, 0.0},
        {35.0 / 384.0, 0.0, 500.0 / 1113.0, 125.0 / 192.0, -2187.0 / 6784.0, 11.0 / 84.0, 0.0},
    }};

    static constexpr std::array<scalar, 7> b{35.0 / 384.0, 0.0, 500.0 / 1113.0, 125.0 / 192.0, -2187.0 / 6784.0,
                                             11.0 / 84.0,  0.0};

    static constexpr std::array<scalar, 7> b_hat{
        5179.0 / 57600.0, 0.0, 7571.0 / 16695.0, 393.0 / 640.0, -92097.0 / 339200.0, 187.0 / 2100.0, 1.0 / 40.0};
};

}  // namespace numsol
