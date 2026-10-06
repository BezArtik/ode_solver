#include <gtest/gtest.h>

#include <cmath>

#include "common/methods.hpp"
#include "numsol/methods/method.hpp"

namespace {

using namespace numsol;
using namespace numsol::tests;

TEST(adaptive, respects_tolerance_1e6) {
    expect_error_below<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-6, .atol_ = 1e-9}, 1e-5);
}

TEST(adaptive, respects_tolerance_1e9) {
    expect_error_below<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-9, .atol_ = 1e-12}, 1e-8);
}

TEST(adaptive, tighter_tolerance_more_steps) {
    auto&& loose = run_oscillator<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-4, .atol_ = 1e-7});
    auto&& tight = run_oscillator<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-8, .atol_ = 1e-11});

    EXPECT_GT(tight.stats_.steps_, loose.stats_.steps_);
}

TEST(adaptive, h0_does_not_affect_accuracy) {
    auto&& err_small = run_and_measure<dopri5>({.h0_ = 1e-6, .adaptive_ = true, .rtol_ = 1e-6, .atol_ = 1e-9});
    auto&& err_large = run_and_measure<dopri5>({.h0_ = 0.5, .adaptive_ = true, .rtol_ = 1e-6, .atol_ = 1e-9});

    EXPECT_LT(err_small, 1e-5);
    EXPECT_LT(err_large, 1e-5);
}

TEST(adaptive, rejects_too_large_initial_step) {
    expect_rejected_above<dopri5>({.h0_ = 1.0, .adaptive_ = true, .rtol_ = 1e-6, .atol_ = 1e-9}, 0);
}

TEST(adaptive, error_scales_with_tolerance) {
    auto&& err_loose = run_and_measure<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-4, .atol_ = 1e-7});
    auto&& err_tight = run_and_measure<dopri5>({.h0_ = 0.01, .adaptive_ = true, .rtol_ = 1e-8, .atol_ = 1e-11});

    EXPECT_LT(err_tight, err_loose);
}

TEST(adaptive, exponential) {
    auto&& p = problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = solver_options{};
    opts.h0_ = 0.1;
    opts.adaptive_ = true;
    opts.rtol_ = 1e-8;
    opts.atol_ = 1e-12;

    auto&& integ = integrator{dopri5{}, opts};
    auto&& sol = integ.run(p);

    EXPECT_TRUE(sol.stats_.success_);
    EXPECT_NEAR(sol.y_.back()[0], std::exp(1.0), 1e-7);
}

}  // namespace
