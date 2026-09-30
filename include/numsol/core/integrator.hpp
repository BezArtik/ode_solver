/**
 * @file integrator.hpp
 * @brief Main integration driver.
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include "numsol/core/errors.hpp"
#include "numsol/core/options.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/core/solution.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

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
 * @brief Drives numerical integration of an initial value problem.
 *
 * Combines a numerical method with a right-hand side and produces a
 * @ref solution on the interval @c [t0, t_end].
 *
 * @tparam Method Method type.
 * @tparam F      Right-hand side type satisfying @ref rhs.
 */
template <typename Method, typename F>
    requires rhs<F> && requires(Method&& m, F&& f, time t, state_view y, time h) {
        { m.step(f, t, y, h) } -> std::convertible_to<step_result>;
        { m.order() } -> std::convertible_to<std::int32_t>;
    }
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
     * @param p Problem to solve.
     * @return  Solution containing all time points and states.
     *
     * @throws invalid_problem_error     if the problem is malformed.
     * @throws max_steps_exceeded_error  if @c max_steps is reached.
     * @throws step_size_too_small_error if @c h falls below @c h_min.
     */
    [[nodiscard]] solution run(const problem<F>& p) {
        if (p.t_end_ < p.t0_) throw invalid_problem_error{"t_end must be >= t0"};
        if (p.y0_.empty()) throw invalid_problem_error{"initial state must be non-empty"};
        if (opts_.h0_ <= 0.0) throw invalid_problem_error{"h0 must be positive"};

        auto&& sol = solution{};

        auto&& span = p.t_end_ - p.t0_;
        if (span > 0.0 && opts_.h0_ > 0.0) {
            auto&& estimated = static_cast<std::size_t>(span / opts_.h0_) + 2;
            sol.t_.reserve(estimated);
            sol.y_.reserve(estimated);
        }

        auto t = p.t0_;
        auto y = p.y0_;
        auto&& h = opts_.h0_;

        auto&& rhs_evals = std::size_t{};

        struct counting_rhs {
            [[nodiscard]] state operator()(time t, state_view y) const {
                ++counter_;
                return f_(t, y);
            }

            const F& f_;
            std::size_t& counter_;
        } count_rhs{p.rhs_, rhs_evals};

        sol.t_.push_back(t);
        sol.y_.push_back(y);

        while (t < p.t_end_) {
            if (sol.stats_.steps_ >= opts_.max_steps_)
                throw max_steps_exceeded_error("maximum number of steps exceeded");

            if (h < opts_.h_min_) throw step_size_too_small_error("step size fell below h_min");

            const auto h_actual = std::min(h, p.t_end_ - t);

            auto&& res = method_.step(count_rhs, t, y, h_actual);

            t = res.t_next_;
            y = std::move(res.y_next_);

            ++sol.stats_.steps_;
            sol.t_.push_back(t);
            sol.y_.push_back(y);

            if (observer_) {
                auto&& action = observer_(t, y);
                if (action == observer_action::stop_) break;
            }
        }

        sol.stats_.rhs_evals_ = rhs_evals;
        sol.stats_.last_h_ = h;
        sol.stats_.success_ = (t >= p.t_end_);

        return sol;
    }

private:
    Method method_;
    solver_options opts_;
    observer observer_;
};

}  // namespace numsol
