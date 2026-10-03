/**
 * @file explicit_rk.hpp
 * @brief Generic explicit Runge-Kutta method.
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

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
    /// Number of stages.
    static constexpr std::size_t stages = Tableau::b.size();

    /**
     * @brief Returns the order of the method.
     */
    [[nodiscard]] std::int32_t order() const noexcept { return Tableau::order; }

    /**
     * @brief Reserves internal buffers for a system of dimension @p n.
     *
     * Called once by the integrator before the integration loop.
     * After this call, @ref step performs no heap allocations.
     */
    void reserve(std::size_t n) {
        if (n == dim_) return;
        dim_ = n;
        for (auto&& k : k_) k.resize(n);
        y_stage_.resize(n);
        y_next_.resize(n);
    }

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
        std::ranges::copy(y, y_stage_.begin());

        for (std::size_t i = 0; i < stages; ++i) {
            if (i > 0) std::ranges::copy(y, y_stage_.begin());

            for (std::size_t j = 0; j < i; ++j) {
                auto&& a_ij = Tableau::a[i][j];
                if (a_ij == scalar{0}) continue;

                auto&& k_j = k_[j];
                for (std::size_t m = 0; m < dim_; ++m) y_stage_[m] += h * a_ij * k_j[m];
            }

            auto&& dy = f(t + Tableau::c[i] * h, y_stage_);
            std::ranges::copy(dy, k_[i].begin());
        }

        std::ranges::copy(y, y_next_.begin());

        for (std::size_t i = 0; i < stages; ++i) {
            auto&& b_i = Tableau::b[i];
            if (b_i == scalar{0}) continue;

            auto&& k_i = k_[i];
            for (std::size_t m = 0; m < dim_; ++m) y_next_[m] += h * b_i * k_i[m];
        }

        return {{y_next_.begin(), y_next_.end()}, t + h};
    }

private:
    std::size_t dim_ = 0;
    std::array<std::vector<scalar>, stages> k_;
    std::vector<scalar> y_stage_;
    std::vector<scalar> y_next_;
};

}  // namespace numsol
