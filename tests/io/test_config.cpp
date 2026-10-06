#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <tuple>

#include "common/temp_file.hpp"
#include "numsol/core/errors.hpp"
#include "numsol/io/config.hpp"

namespace {

using namespace numsol;
using namespace numsol::app;
using namespace numsol::tests;

TEST_F(temp_file_test, parses_minimal) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0, 0.0]

[problem.rhs]
y1 = "y2"
y2 = "-y1"
)");

    auto&& cfg = load_config(path_.string());

    EXPECT_DOUBLE_EQ(cfg.t0_, 0.0);
    EXPECT_DOUBLE_EQ(cfg.t_end_, 1.0);
    ASSERT_EQ(cfg.y0_.size(), 2);
    EXPECT_DOUBLE_EQ(cfg.y0_[0], 1.0);
    EXPECT_DOUBLE_EQ(cfg.y0_[1], 0.0);
    ASSERT_EQ(cfg.rhs_expressions_.size(), 2);
    EXPECT_EQ(cfg.rhs_expressions_[0], "y2");
    EXPECT_EQ(cfg.rhs_expressions_[1], "-y1");
    EXPECT_EQ(cfg.method_, "rk4");
    EXPECT_TRUE(cfg.params_.empty());
    EXPECT_TRUE(cfg.output_path_.empty());
    EXPECT_EQ(cfg.output_format_, "csv");
}

TEST_F(temp_file_test, parses_full) {
    write(R"(
[problem]
t0    = 0.5
t_end = 5.0
y0    = [1.0, 2.0]

[problem.rhs]
y1 = "y2"
y2 = "-omega * omega * y1"

[problem.params]
omega = 2.0

[solver]
method    = "rk2"
h0        = 0.01
h_min     = 1e-10
h_max     = 0.5
max_steps = 10000

[output]
path   = "out/test.csv"
format = "csv"
)");

    auto&& cfg = load_config(path_.string());

    EXPECT_DOUBLE_EQ(cfg.t0_, 0.5);
    EXPECT_DOUBLE_EQ(cfg.t_end_, 5.0);
    ASSERT_EQ(cfg.y0_.size(), 2);
    ASSERT_EQ(cfg.rhs_expressions_.size(), 2);
    ASSERT_EQ(cfg.params_.size(), 1);
    EXPECT_DOUBLE_EQ(cfg.params_.at("omega"), 2.0);
    EXPECT_EQ(cfg.method_, "rk2");
    EXPECT_DOUBLE_EQ(cfg.opts_.h0_, 0.01);
    EXPECT_DOUBLE_EQ(cfg.opts_.h_min_, 1e-10);
    EXPECT_DOUBLE_EQ(cfg.opts_.h_max_, 0.5);
    EXPECT_EQ(cfg.opts_.max_steps_, 10000);
    EXPECT_EQ(cfg.output_path_, "out/test.csv");
    EXPECT_EQ(cfg.output_format_, "csv");
}

TEST_F(temp_file_test, params_optional) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"
)");

    auto&& cfg = load_config(path_.string());

    EXPECT_TRUE(cfg.params_.empty());
}

TEST_F(temp_file_test, solver_optional) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"
)");

    auto&& cfg = load_config(path_.string());

    EXPECT_EQ(cfg.method_, "rk4");
    EXPECT_DOUBLE_EQ(cfg.opts_.h0_, 1e-3);
}

TEST_F(temp_file_test, output_optional) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"
)");

    auto&& cfg = load_config(path_.string());

    EXPECT_TRUE(cfg.output_path_.empty());
    EXPECT_EQ(cfg.output_format_, "csv");
}

TEST_F(temp_file_test, rhs_order_independent) {
    write(R"(
[problem]
t_end = 1.0
y0    = [1.0, 2.0, 3.0]

[problem.rhs]
y3 = "y1"
y1 = "y2"
y2 = "y3"
)");

    auto&& cfg = load_config(path_.string());

    ASSERT_EQ(cfg.rhs_expressions_.size(), 3u);
    EXPECT_EQ(cfg.rhs_expressions_[0], "y2");
    EXPECT_EQ(cfg.rhs_expressions_[1], "y3");
    EXPECT_EQ(cfg.rhs_expressions_[2], "y1");
}

TEST_F(temp_file_test, rejects_missing_file) {
    auto&& missing = std::filesystem::temp_directory_path() / "numsol_does_not_exist_12345.toml";
    EXPECT_THROW(std::ignore = load_config(missing.string()), invalid_problem_error);
}

class config_error_test : public temp_file_test, public ::testing::WithParamInterface<std::string_view> {};

TEST_P(config_error_test, rejects_invalid_input) {
    write(GetParam());
    EXPECT_THROW(std::ignore = load_config(path_string()), invalid_problem_error);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(malformed_configs, 
        config_error_test,
::testing::Values(
// Invalid TOML syntax.
"this is not valid toml ===",

// Missing [problem] section.
R"(
[solver]
method = "rk4"
)",

// Missing t_end.
R"(
[problem]
y0 = [1.0]

[problem.rhs]
y1 = "-y1"
)",

// Missing y0.
R"(
[problem]
t_end = 1.0

[problem.rhs]
y1 = "-y1"
)",

// Missing rhs.
R"(
[problem]
t_end = 1.0
y0    = [1.0]
)",

// Size mismatch between y0 and rhs.
R"(
[problem]
t_end = 1.0
y0    = [1.0, 2.0]

[problem.rhs]
y1 = "-y1"
)",

// Wrong type: t_end is a string.
R"(
[problem]
t_end = "one"
y0    = [1.0]

[problem.rhs]
y1 = "-y1"
)",

// Wrong type: y0 is not an array.
R"(
[problem]
t_end = 1.0
y0    = "not an array"

[problem.rhs]
y1 = "-y1"
)",

// y0 contains a non-numeric element.
R"(
[problem]
t_end = 1.0
y0    = [1.0, "two"]

[problem.rhs]
y1 = "-y1"
y2 = "-y2"
)",

// rhs value is not a string.
R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = 42
)",

// params value is not a number.
R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"

[problem.params]
alpha = "not a number"
)",

// Negative max_steps.
R"(
[problem]
t_end = 1.0
y0    = [1.0]

[problem.rhs]
y1 = "-y1"

[solver]
max_steps = -1
)",

// Non-contiguous rhs indices.
R"(
[problem]
t_end = 1.0
y0    = [1.0, 2.0]

[problem.rhs]
y1 = "-y1"
y3 = "-y3"
)"));
// clang-format on

}  // namespace
