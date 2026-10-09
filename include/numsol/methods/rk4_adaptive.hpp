/**
 * @file rk4_adaptive.hpp
 * @brief Fourth-order Runge-Kutta with double-step error control.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "numsol/core/options.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Fourth-order Runge-Kutta with double-step error control.
 *
 * At each step the method computes two approximations:
 * - a coarse one with a single step of size @c h,
 * - a fine one with two half-steps of size @c h/2.
 *
 * The difference @c |v_i - v_2i| provides the raw local error
 * estimate (OLP). If it exceeds @ref solver_options::eps_, the step
 * is rejected and the step size is halved. If it is well below the
 * threshold, the step is accepted and the step size is doubled.
 * Otherwise the step is accepted with the same step size.
 */
class rk4_adaptive {
public:
    static constexpr std::size_t stages = 4;

    /// Order of the method.
    [[nodiscard]] std::int32_t order() const noexcept { return 4; }

    /// Reserves internal buffers for a system of dimension @p n.
    void reserve(std::size_t n) {
        if (n == dim_) return;
        dim_ = n;
        for (auto&& k : k_) k.resize(n);
        y_stage_.resize(n);
    }

    /**
     * @brief Performs one integration step.
     *
     * @tparam F Right-hand side type satisfying @ref rhs.
     * @param f   Right-hand side function.
     * @param t   Current time.
     * @param y   Current state.
     * @param h   Step size.
     * @param opts Solver options; @c opts.eps_ is used as the
     *             local-error threshold.
     * @return  Step result with @ref y_coarse_, @ref olp_,
     *          @ref halved_, and @ref doubled_ populated.
     */
    template <rhs F>
    [[nodiscard]] step_result step(F&& f, time t, state_view y, time h, const solver_options& opts) {
        auto&& eps = opts.eps_;

        auto&& y_coarse = rk4_full_step(f, t, y, h);

        auto&& y_mid = rk4_full_step(f, t, y, h * 0.5);
        auto&& y_fine = rk4_full_step(f, t + h * 0.5, y_mid, h * 0.5);

        auto&& lee = scalar{};
        for (std::size_t m = 0; m < dim_; ++m) lee = std::max(lee, std::abs(y_coarse[m] - y_fine[m]));

        auto&& res = step_result{};
        res.y_coarse_ = std::move(y_coarse);
        res.y_next_ = std::move(y_fine);
        res.t_next_ = t + h;
        res.lee_ = lee;

        if (lee > eps) {
            res.accepted_ = false;
            res.suggested_h_ = h * 0.5;
            res.halved_ = true;
        } else if (lee < eps / 32.0) {
            res.accepted_ = true;
            res.suggested_h_ = h * 2.0;
            res.doubled_ = true;
        } else {
            res.accepted_ = true;
            res.suggested_h_ = h;
        }

        return res;
    }

private:
    template <rhs F>
    [[nodiscard]] state rk4_full_step(F&& f, time t, state_view y, time h) {
        std::ranges::copy(y, y_stage_.begin());
        {
            auto&& dy = f(t, y_stage_);
            std::ranges::copy(dy, k_[0].begin());
        }

        std::ranges::copy(y, y_stage_.begin());
        for (std::size_t m = 0; m < dim_; ++m) y_stage_[m] += (h * 0.5) * k_[0][m];

        {
            auto dy = f(t + h * 0.5, y_stage_);
            std::ranges::copy(dy, k_[1].begin());
        }

        std::ranges::copy(y, y_stage_.begin());
        for (std::size_t m = 0; m < dim_; ++m) y_stage_[m] += (h * 0.5) * k_[1][m];

        {
            auto&& dy = f(t + h * 0.5, y_stage_);
            std::ranges::copy(dy, k_[2].begin());
        }

        std::ranges::copy(y, y_stage_.begin());
        for (std::size_t m = 0; m < dim_; ++m) y_stage_[m] += h * k_[2][m];

        {
            auto&& dy = f(t + h, y_stage_);
            std::ranges::copy(dy, k_[3].begin());
        }

        auto&& result = state(dim_);
        auto&& h6 = h / 6.0;
        for (std::size_t m = 0; m < dim_; ++m)
            result[m] = y[m] + h6 * (k_[0][m] + 2.0 * k_[1][m] + 2.0 * k_[2][m] + k_[3][m]);

        return result;
    }

    std::size_t dim_ = 0;
    std::array<std::vector<scalar>, stages> k_;
    std::vector<scalar> y_stage_;
};

}  // namespace numsol
