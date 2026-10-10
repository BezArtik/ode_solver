/**
 * @file adaptive_rk.hpp
 * @brief Generic adaptive Runge-Kutta method.
 */

#pragma once

#include "numsol/core/options.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>

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

        for (std::size_t m = 0; m < dim_; ++m) {
            auto&& acc = scalar{};
            for (std::size_t i = 0; i < stages; ++i) acc += Tableau::b_hat[i] * k_[i][m];
            y_hat_[m] = y[m] + h * acc;
        }

        auto&& err_sq = scalar{};
        auto&& raw_lee = scalar{};
        for (std::size_t m = 0; m < dim_; ++m) {
            auto&& diff = y_next_[m] - y_hat_[m];
            raw_lee = std::max(raw_lee, std::abs(diff));

            auto&& sc = opts.atol_ + opts.rtol_ * std::max(std::abs(y[m]), std::abs(y_next_[m]));
            auto&& e = diff / sc;
            err_sq += e * e;
        }

        auto&& err = std::sqrt(err_sq / static_cast<scalar>(dim_));

        auto&& res = step_result{};
        res.y_next_ = y_next_;
        res.t_next_ = t + h;
        res.error_estimate_ = err;
        res.lee_ = raw_lee;

        if (err > 1.0) {
            res.accepted_ = false;
            res.suggested_h_ = h * 0.5;
            res.halved_ = true;
        } else if (err < 1.0 / 32.0) {
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
    static constexpr std::size_t stages = Tableau::stages;
    std::size_t dim_ = 0;
    std::array<state, stages> k_;
    state y_stage_;
    state y_next_;
    state y_hat_;
};

}  // namespace numsol
