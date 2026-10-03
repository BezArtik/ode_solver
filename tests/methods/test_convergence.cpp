#include <gtest/gtest.h>

#include <cmath>

#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/methods/euler.hpp"
#include "numsol/methods/rk2.hpp"
#include "numsol/methods/rk3.hpp"
#include "numsol/methods/rk4.hpp"

namespace {

struct oscillator {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view y) const { return {y[1], -y[0]}; }
};

template <typename Method>
auto run_and_measure(numsol::scalar h) {
    auto&& p = numsol::problem{oscillator{}, {1.0, 0.0}, 0.0, 1.0};

    auto&& opts = numsol::solver_options{};
    opts.h0_ = h;

    auto&& integ = numsol::integrator{Method{}, opts};
    auto&& sol = integ.run(p);

    auto&& best = numsol::scalar{};
    for (std::size_t i = 0; i < sol.t_.size(); ++i) {
        auto&& err = std::abs(sol.y_[i][0] - std::cos(sol.t_[i]));
        if (err > best) best = err;
    }
    return best;
}

template <typename Method>
void expect_order(numsol::scalar expected_ratio, numsol::scalar tolerance) {
    auto&& err_coarse = run_and_measure<Method>(0.1);
    auto&& err_fine = run_and_measure<Method>(0.05);
    auto&& ratio = err_coarse / err_fine;

    EXPECT_GT(ratio, expected_ratio - tolerance);
    EXPECT_LT(ratio, expected_ratio + tolerance);
}

TEST(convergence, euler_order_1) {
    expect_order<numsol::euler>(2.0, 0.5);
}

TEST(convergence, rk2_order_2) {
    expect_order<numsol::rk2>(4.0, 0.5);
}

TEST(convergence, rk3_order_3) {
    expect_order<numsol::rk3>(8.0, 1.0);
}

TEST(convergence, rk4_order_4) {
    expect_order<numsol::rk4>(16.0, 2.0);
}

TEST(convergence, rk4_abs_error_small) {
    auto&& err = run_and_measure<numsol::rk4>(0.01);
    EXPECT_LT(err, 1e-7);
}

}  // namespace
