#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <string>

#include "common/temp_file.hpp"
#include "numsol/core/integrator.hpp"
#include "numsol/core/problem.hpp"
#include "numsol/expr/expr_rhs.hpp"
#include "numsol/io/config.hpp"
#include "numsol/io/solution_writer.hpp"
#include "numsol/methods/method.hpp"

namespace {

using namespace numsol::tests;

TEST_F(temp_file_test, oscillator_full_pipeline) {
    write(R"(
[problem]
t0    = 0.0
t_end = 1.0
y0    = [1.0, 0.0]

[problem.rhs]
y1 = "y2"
y2 = "-y1"

[solver]
method = "rk4"
h0     = 0.01
)");

    auto&& cfg = numsol::app::load_config(path_string());

    auto&& rhs = numsol::app::expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = numsol::problem{std::move(rhs), cfg.y0_, cfg.t0_, cfg.t_end_};

    auto&& integ = numsol::integrator{numsol::rk4{}, cfg.opts_};
    auto&& sol = integ.run(p);

    ASSERT_FALSE(sol.t_.empty());
    EXPECT_DOUBLE_EQ(sol.t_.back(), 1.0);
    EXPECT_TRUE(sol.stats_.success_);

    EXPECT_NEAR(sol.y_.back()[0], std::cos(1.0), 1e-7);
    EXPECT_NEAR(sol.y_.back()[1], -std::sin(1.0), 1e-7);

    auto&& out = std::ostringstream{};
    numsol::app::write_solution_csv(sol, out);

    auto&& text = out.str();
    EXPECT_FALSE(text.empty());
    EXPECT_EQ(text.substr(0, text.find('\n')), "t,y1,y2");

    auto&& last_line_start = text.find_last_of('\n', text.size() - 2);
    auto&& last_line = text.substr(last_line_start + 1);
    EXPECT_NE(last_line.find("1,"), std::string::npos);
}

TEST_F(temp_file_test, exponential_full_pipeline) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "y1"
)");

    auto&& cfg = numsol::app::load_config(path_string());
    auto&& rhs = numsol::app::expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = numsol::problem{std::move(rhs), cfg.y0_, cfg.t0_, cfg.t_end_};

    auto&& integ = numsol::integrator{numsol::rk4{}, cfg.opts_};
    auto&& sol = integ.run(p);

    EXPECT_NEAR(sol.y_.back()[0], std::exp(1.0), 1e-7);
}

TEST_F(temp_file_test, lorenz_does_not_crash) {
    write(R"toml(
[problem]
t0    = 0.0
t_end = 1.0
y0    = [1.0, 1.0, 1.0]

[problem.rhs]
y1 = "sigma * (y2 - y1)"
y2 = "y1 * (rho - y3) - y2"
y3 = "y1 * y2 - beta * y3"

[problem.params]
sigma = 10.0
rho   = 28.0
beta  = 2.6666666666666665

[solver]
method = "rk4"
h0     = 0.001
)toml");

    auto&& cfg = numsol::app::load_config(path_string());
    auto&& rhs = numsol::app::expr_rhs{cfg.rhs_expressions_, cfg.params_};

    auto&& p = numsol::problem{std::move(rhs), cfg.y0_, cfg.t0_, cfg.t_end_};

    auto&& integ = numsol::integrator{numsol::rk4{}, cfg.opts_};
    auto&& sol = integ.run(p);

    ASSERT_FALSE(sol.t_.empty());
    EXPECT_DOUBLE_EQ(sol.t_.back(), 1.0);
    EXPECT_TRUE(sol.stats_.success_);

    for (auto&& y : sol.y_) {
        for (auto&& value : y) {
            EXPECT_TRUE(std::isfinite(value));
        }
    }
}

TEST_F(temp_file_test, unknown_method_throws) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"

[solver]
method = "rk99"
)");

    auto&& cfg = numsol::app::load_config(path_string());

    auto&& it = std::ranges::find(numsol::method_table, cfg.method_, &numsol::method_entry::first);
    EXPECT_EQ(it, numsol::method_table.end());
}

}  // namespace
