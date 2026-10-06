#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>

#include "common/math.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"

namespace numsol::tests {

struct run_config {
    scalar h0_ = 0.01;
    scalar t_end_ = 1.0;
    bool adaptive_ = false;
    scalar rtol_ = 1e-6;
    scalar atol_ = 1e-9;
};

template <typename Method>
auto run_oscillator(const run_config& cfg) {
    auto&& p = problem{oscillator{}, {1.0, 0.0}, 0.0, cfg.t_end_};

    auto&& opts = solver_options{};
    opts.h0_ = cfg.h0_;
    opts.adaptive_ = cfg.adaptive_;
    opts.rtol_ = cfg.rtol_;
    opts.atol_ = cfg.atol_;

    auto&& integ = integrator{Method{}, opts};
    return integ.run(p);
}

template <typename Method>
auto run_and_measure(const run_config& cfg) {
    auto&& sol = run_oscillator<Method>(cfg);
    auto&& best = scalar{};
    for (std::size_t i = 0; i < sol.t_.size(); ++i) {
        auto&& err = std::abs(sol.y_[i][0] - std::cos(sol.t_[i]));
        if (err > best) best = err;
    }
    return best;
}

template <typename Method>
void expect_error_below(const run_config& cfg, scalar threshold) {
    auto&& err = run_and_measure<Method>(cfg);
    EXPECT_LT(err, threshold);
}

template <typename Method>
void expect_steps_above(const run_config& cfg, std::size_t threshold) {
    auto&& sol = run_oscillator<Method>(cfg);
    EXPECT_GT(sol.stats_.steps_, threshold);
}

template <typename Method>
void expect_rejected_above(const run_config& cfg, std::size_t threshold) {
    auto&& sol = run_oscillator<Method>(cfg);
    EXPECT_GT(sol.stats_.rejected_, threshold);
}

template <typename Method>
void expect_success(const run_config& cfg) {
    auto&& sol = run_oscillator<Method>(cfg);
    EXPECT_TRUE(sol.stats_.success_);
}

template <typename Method>
void expect_order(scalar expected_ratio, scalar tolerance) {
    auto&& err_coarse = run_and_measure<Method>({.h0_ = 0.1});
    auto&& err_fine = run_and_measure<Method>({.h0_ = 0.05});
    auto&& ratio = err_coarse / err_fine;

    EXPECT_GT(ratio, expected_ratio - tolerance);
    EXPECT_LT(ratio, expected_ratio + tolerance);
}

}  // namespace numsol::tests
