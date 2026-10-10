/**
 * @file adaptive_rk.hpp
 * @brief Generic adaptive Runge-Kutta method.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "numsol/core/options.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

namespace numsol {

/**
 * @brief Adaptive Runge-Kutta method with an embedded error estimate.
 *
 * The tableau must provide @c c, @c a, @c b, @c b_hat, @c order,
 * and @c stages.
 *
 * @tparam Tableau Butcher tableau type.
 */
template <typename Tableau>
class adaptive_rk {
public:
    template <rhs F>
    [[nodiscard]] step_result step(F&& f, time t, state_view y, time h, const solver_options& opts) {
        if (dim_ != y.size()) {
            dim_ = y.size();
            for (auto&& k : k_) k.resize(dim_);
            y_stage_.resize(dim_);
            y_next_.resize(dim_);
            y_hat_.resize(dim_);
        }

        for (std::size_t i = 0; i < stages; ++i) {
            std::ranges::copy(y, y_stage_.begin());

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

        std::ranges::copy(y, y_hat_.begin());
        for (std::size_t i = 0; i < stages; ++i) {
            auto&& b_i = Tableau::b_hat[i];
            if (b_i == scalar{0}) continue;
            auto&& k_i = k_[i];
            for (std::size_t m = 0; m < dim_; ++m) y_hat_[m] += h * b_i * k_i[m];
        }

        auto&& err_sq = scalar{};
        for (std::size_t m = 0; m < dim_; ++m) {
            auto&& sc = opts.atol_ + opts.rtol_ * std::max(std::abs(y[m]), std::abs(y_next_[m]));
            auto&& e = (y_next_[m] - y_hat_[m]) / sc;
            err_sq += e * e;
        }

        constexpr auto safety = scalar{0.9};
        constexpr auto exp = scalar{1.0 / static_cast<scalar>(Tableau::order)};
        auto&& err = std::sqrt(err_sq / static_cast<scalar>(dim_));
        auto&& h_new = h * std::clamp(safety * std::pow(1.0 / err, exp), 0.2, 5.0);

        return {
            .y_next_ = y_next_,
            .t_next_ = t + h,
            .error_estimate_ = err,
            .suggested_h_ = h_new,
            .accepted_ = (err <= 1.0),
        };
    }

private:
    static constexpr std::size_t stages = Tableau::stages;
    std::size_t dim_ = 0;
    std::array<std::vector<scalar>, stages> k_;
    state y_stage_;
    state y_next_;
    state y_hat_;
};

}  // namespace numsol
