/**
 * @file integrator.hpp
 * @brief Main integration driver.
 */

#pragma once

#include "numsol/core/errors.hpp"
#include "numsol/core/options.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/solution.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

namespace numsol {

/**
 * @brief Action returned by an observer after each step.
 */
enum class observer_action : std::uint8_t {
    /// Continue integration.
    continue_,

    /// Stop integration immediately.
    stop_,
};

/**
 * @brief Callback invoked after each accepted step.
 *
 * Receives the current time and state. The return value controls
 * whether integration continues.
 */
using observer = std::function<observer_action(time, state_view)>;

/**
 * @brief Requirements for a fixed-step method.
 *
 * A fixed-step method provides @c order(), @c reserve(n), and a
 * @c step() callable that takes the RHS, time, state, and step size.
 */
template <typename Method, typename F>
concept fixed_step_method = rhs<F> && requires(Method&& m, F&& f, time t, state_view y, time h) {
    { m.step(f, t, y, h) } -> std::convertible_to<step_result>;
};

/**
 * @brief Requirements for an adaptive-step method.
 *
 * An adaptive-step method provides @c order(), @c reserve(n), and a
 * @c step() callable that additionally receives relative and absolute
 * tolerances.
 */
template <typename Method, typename F>
concept adaptive_step_method =
    rhs<F> && requires(Method&& m, F&& f, time t, state_view y, time h, const solver_options& opts) {
        { m.step(f, t, y, h, opts) } -> std::convertible_to<step_result>;
    };

/**
 * @brief Either kind of method.
 */
template <typename Method, typename F>
concept usable_method = fixed_step_method<Method, F> || adaptive_step_method<Method, F>;

/**
 * @brief Drives numerical integration of an initial value problem.
 *
 * Combines a numerical method with a right-hand side and produces a
 * @ref solution on the interval @c [t0, t_end].
 *
 * @tparam Method Method type.
 */
template <typename Method>
class integrator {
public:
    /**
     * @brief Constructs an integrator.
     *
     * @param m   Numerical method.
     * @param opts Solver options.
     * @param obs Observer called after each step; may be empty.
     */
    integrator(Method m, solver_options opts, observer obs = {})
        : method_{std::move(m)}, opts_{std::move(opts)}, observer_{std::move(obs)} {}

    /**
     * @brief Runs integration.
     *
     * @tparam F Right-hand side type satisfying @ref rhs.
     * @param p Problem to solve.
     * @return  Solution containing all time points and states.
     *
     * @throws invalid_problem_error     if the problem is malformed.
     * @throws max_steps_exceeded_error  if @c max_steps is reached.
     * @throws step_size_too_small_error if @c h falls below @c h_min.
     */
    template <rhs F>
        requires usable_method<Method, F>
    [[nodiscard]] solution run(const problem<F>& p) {
        if (p.t_end_ < p.t0_) throw invalid_problem_error{"t_end must be >= t0"};
        if (p.y0_.empty()) throw invalid_problem_error{"initial state must be non-empty"};
        if (opts_.h0_ <= 0.0) throw invalid_problem_error{"h0 must be positive"};

        auto&& sol = solution{};
        auto&& stats = solver_stats{};

        auto&& span = p.t_end_ - p.t0_;
        if (span > 0.0 && opts_.h0_ > 0.0) {
            auto&& estimated = static_cast<std::size_t>(span / opts_.h0_) + 2;
            sol.t_.reserve(estimated);
            sol.y_.reserve(estimated);
        }

        auto t = p.t0_;
        auto y = p.y0_;
        auto h = opts_.h0_;

        auto&& rhs_evals = std::size_t{};
        auto&& p_rhs = p.rhs_;
        auto count_rhs = [&p_rhs, &rhs_evals](time t, state_view y) {
            ++rhs_evals;
            return p_rhs(t, y);
        };

        sol.t_.push_back(t);
        sol.y_.push_back(y);

        while (t < p.t_end_) {
            if (stats.steps_ + stats.rejected_ >= opts_.max_steps_)
                throw max_steps_exceeded_error{"maximum number of steps exceeded"};

            if (h < opts_.h_min_) throw step_size_too_small_error{"step size fell below h_min"};

            const auto h_actual = std::min(h, p.t_end_ - t);

            if constexpr (adaptive_step_method<Method, F>) {
                auto&& res = method_.step(count_rhs, t, y, h_actual, opts_);

                if (res.lee_ > stats.max_lee_) {
                    stats.max_lee_ = res.lee_;
                    stats.max_lee_at_ = t;
                }
                if (stats.min_h_ == time{} || h_actual < stats.min_h_) {
                    stats.min_h_ = h_actual;
                    stats.min_h_at_ = t;
                }
                if (h_actual > stats.max_h_) {
                    stats.max_h_ = h_actual;
                    stats.max_h_at_ = t;
                }

                if (!res.accepted_) {
                    ++stats.rejected_;
                    if (res.halved_) ++stats.halvings_;
                    h = res.suggested_h_;
                    continue;
                }

                if (res.doubled_) ++stats.doublings_;
                t = res.t_next_;
                y = std::move(res.y_next_);

                if (res.suggested_h_ > time{}) h = std::clamp(res.suggested_h_, opts_.h_min_, opts_.h_max_);

                if (!res.y_coarse_.empty()) {
                    sol.y_coarse_.push_back(std::move(res.y_coarse_));
                    sol.lee_.push_back(res.lee_);
                    sol.h_.push_back(h_actual);
                    sol.c1_.push_back(stats.halvings_);
                    sol.c2_.push_back(stats.doublings_);
                }
            } else {
                auto&& res = method_.step(count_rhs, t, y, h_actual);

                if (stats.min_h_ == time{} || h_actual < stats.min_h_) {
                    stats.min_h_ = h_actual;
                    stats.min_h_at_ = t;
                }
                if (h_actual > stats.max_h_) {
                    stats.max_h_ = h_actual;
                    stats.max_h_at_ = t;
                }
                t = res.t_next_;
                y = std::move(res.y_next_);
            }

            ++stats.steps_;
            sol.t_.push_back(t);
            sol.y_.push_back(y);

            if (observer_) {
                auto&& action = observer_(t, y);
                if (action == observer_action::stop_) break;
            }
        }

        stats.rhs_evals_ = rhs_evals;
        stats.last_h_ = h;
        stats.success_ = (t >= p.t_end_);
        sol.stats_ = std::move(stats);

        return sol;
    }

private:
    Method method_;
    solver_options opts_;
    observer observer_;
};

}  // namespace numsol
