/**
 * @file explicit_rk.hpp
 * @brief Generic explicit Runge-Kutta method.
 */

#pragma once

#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <utility>

namespace numsol {

/**
 * @brief Explicit Runge-Kutta method defined by a Butcher tableau.
 *
 * The tableau type must provide:
 * - @c stages — number of stages,
 * - @c c — nodes,
 * - @c a — coefficient matrix (lower triangular),
 * - @c b — weights,
 * - @c order — order of the method.
 *
 * All members must be @c static @c constexpr.
 *
 * @tparam Tableau Butcher tableau type.
 */
template <typename Tableau>
class explicit_rk {
public:
    /**
     * @brief Performs one integration step.
     *
     * @tparam F Right-hand side type satisfying @ref rhs.
     * @param f Right-hand side function.
     * @param t Current time.
     * @param y Current state.
     * @param h Step size.
     * @return  State and time after the step.
     */
    template <rhs F>
    [[nodiscard]] step_result step(F&& f, time t, state_view y, time h) {
        if (dim_ != y.size()) {
            dim_ = y.size();
            for (auto&& k : k_) k.resize(dim_);
            y_stage_.resize(dim_);
            y_next_.resize(dim_);
        }

        for (std::size_t i = 0; i < stages; ++i) {
            for (std::size_t m = 0; m < dim_; ++m) {
                auto&& acc = scalar{};
                for (std::size_t j = 0; j < i; ++j) acc += Tableau::a[i][j] * k_[j][m];
                y_stage_[m] = y[m] + h * acc;
            }
            k_[i] = std::invoke(std::forward<F>(f), t + Tableau::c[i] * h, y_stage_);
        }

        for (std::size_t m = 0; m < dim_; ++m) {
            auto&& acc = scalar{};
            for (std::size_t i = 0; i < stages; ++i) acc += Tableau::b[i] * k_[i][m];
            y_next_[m] = y[m] + h * acc;
        }

        return {y_next_, t + h};
    }

private:
    static constexpr std::size_t stages = Tableau::stages;
    std::size_t dim_ = 0;
    std::array<state, stages> k_;
    state y_stage_;
    state y_next_;
};

}  // namespace numsol
