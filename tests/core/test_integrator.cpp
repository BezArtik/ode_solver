#include <gtest/gtest.h>

#include <cmath>
#include <tuple>

#include "numsol/core/errors.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/methods/euler.hpp"
#include "numsol/methods/rk2.hpp"
#include "numsol/methods/rk3.hpp"
#include "numsol/methods/rk4.hpp"

namespace {

struct exponential {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view y) const { return {y[0]}; }
};

struct empty_rhs {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view /*y*/) const { return {}; }
};

struct oscillator {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view y) const { return {y[1], -y[0]}; }
};

TEST(integrator, solves_exponential) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.001;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    auto&& sol = integ.run(p);

    ASSERT_FALSE(sol.t_.empty());
    ASSERT_FALSE(sol.y_.empty());
    EXPECT_EQ(sol.t_.size(), sol.y_.size());
    EXPECT_DOUBLE_EQ(sol.t_.front(), 0.0);
    EXPECT_DOUBLE_EQ(sol.t_.back(), 1.0);
    EXPECT_NEAR(sol.y_.back()[0], std::exp(1.0), 1e-10);
    EXPECT_TRUE(sol.stats_.success_);
    EXPECT_GT(sol.stats_.steps_, 0);
    EXPECT_GT(sol.stats_.rhs_evals_, 0);
}

TEST(integrator, includes_initial_point) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    auto&& sol = integ.run(p);

    ASSERT_GE(sol.t_.size(), 1);
    EXPECT_DOUBLE_EQ(sol.t_.front(), 0.0);
    EXPECT_DOUBLE_EQ(sol.y_.front()[0], 1.0);
}

TEST(integrator, truncates_last_step) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.3;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    auto&& sol = integ.run(p);

    EXPECT_DOUBLE_EQ(sol.t_.back(), 1.0);
}

TEST(integrator, zero_interval) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 0.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    auto&& sol = integ.run(p);

    ASSERT_EQ(sol.t_.size(), 1);
    ASSERT_EQ(sol.y_.size(), 1);
    EXPECT_DOUBLE_EQ(sol.t_[0], 0.0);
    EXPECT_DOUBLE_EQ(sol.y_[0][0], 1.0);
    EXPECT_EQ(sol.stats_.steps_, 0);
    EXPECT_TRUE(sol.stats_.success_);
}

TEST(integrator, oscillator_energy_preserved) {
    auto&& p = numsol::problem{oscillator{}, {1.0, 0.0}, 0.0, 10.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.01;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    auto&& sol = integ.run(p);

    for (auto&& y : sol.y_) {
        auto&& energy = y[0] * y[0] + y[1] * y[1];
        EXPECT_NEAR(energy, 1.0, 1e-8);
    }
}

TEST(integrator, throws_on_negative_interval) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 1.0, 0.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    EXPECT_THROW(std::ignore = integ.run(p), numsol::invalid_problem_error);
}

TEST(integrator, throws_on_empty_initial_state) {
    auto&& p = numsol::problem{empty_rhs{}, {}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    EXPECT_THROW(std::ignore = integ.run(p), numsol::invalid_problem_error);
}

TEST(integrator, throws_on_non_positive_step) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.0;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    EXPECT_THROW(std::ignore = integ.run(p), numsol::invalid_problem_error);
}

TEST(integrator, throws_on_max_steps_exceeded) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 100.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 1e-3;
    opts.max_steps_ = 10;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    EXPECT_THROW(std::ignore = integ.run(p), numsol::max_steps_exceeded_error);
}

TEST(integrator, throws_on_step_size_too_small) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 1e-3;
    opts.h_min_ = 1.0;

    auto&& integ = numsol::integrator{numsol::rk4{}, opts};
    EXPECT_THROW(std::ignore = integ.run(p), numsol::step_size_too_small_error);
}

TEST(integrator, observer_called_each_step) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& calls = std::size_t{};
    numsol::observer obs = [&calls](numsol::time /*t*/, numsol::state_view /*y*/) {
        ++calls;
        return numsol::observer_action::continue_;
    };

    auto&& integ = numsol::integrator{numsol::rk4{}, opts, obs};
    auto&& sol = integ.run(p);

    EXPECT_EQ(calls, sol.stats_.steps_);
}

TEST(integrator, observer_can_stop) {
    auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = 0.1;

    auto&& calls = std::size_t{};
    numsol::observer obs = [&calls](numsol::time /*t*/, numsol::state_view /*y*/) {
        ++calls;
        if (calls >= 3) return numsol::observer_action::stop_;
        return numsol::observer_action::continue_;
    };

    auto&& integ = numsol::integrator{numsol::rk4{}, opts, obs};
    auto&& sol = integ.run(p);

    EXPECT_EQ(calls, 3);
    EXPECT_EQ(sol.stats_.steps_, 3);
    EXPECT_FALSE(sol.stats_.success_);
    EXPECT_LT(sol.t_.back(), 1.0);
}

TEST(integrator, all_methods_reach_t_end) {
    const auto run = [](auto method) {
        auto&& p = numsol::problem{exponential{}, {1.0}, 0.0, 1.0};

        auto&& opts = numsol::solver_options{};
        opts.h0_ = 0.01;

        auto&& integ = numsol::integrator{method, opts};
        auto&& sol = integ.run(p);

        EXPECT_DOUBLE_EQ(sol.t_.back(), 1.0);
        EXPECT_TRUE(sol.stats_.success_);
    };

    run(numsol::euler{});
    run(numsol::rk2{});
    run(numsol::rk3{});
    run(numsol::rk4{});
}

}  // namespace
