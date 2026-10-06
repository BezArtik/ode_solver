#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "numsol/core/errors.hpp"
#include "numsol/expr/expr_rhs.hpp"

namespace {

using namespace numsol;
using namespace numsol::app;

expr_rhs make_rhs(std::vector<std::string> expressions, std::unordered_map<std::string, scalar> params = {}) {
    return {std::move(expressions), std::move(params)};
}

TEST(expr_rhs, constant_derivative) {
    auto&& rhs = make_rhs({"0.0"});
    auto&& y = state{1.0};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 1);
    EXPECT_DOUBLE_EQ(dy[0], 0.0);
}

TEST(expr_rhs, identity) {
    auto&& rhs = make_rhs({"y1"});
    auto&& y = state{3.5};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 1);
    EXPECT_DOUBLE_EQ(dy[0], 3.5);
}

TEST(expr_rhs, uses_time) {
    auto&& rhs = make_rhs({"t"});
    auto&& y = state{0.0};
    auto&& dy = rhs(2.5, y);

    ASSERT_EQ(dy.size(), 1);
    EXPECT_DOUBLE_EQ(dy[0], 2.5);
}

TEST(expr_rhs, two_components) {
    auto&& rhs = make_rhs({"y2", "-y1"});
    auto&& y = state{1.0, 2.0};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 2);
    EXPECT_DOUBLE_EQ(dy[0], 2.0);
    EXPECT_DOUBLE_EQ(dy[1], -1.0);
}

TEST(expr_rhs, uses_params) {
    auto&& rhs = make_rhs({"-omega * omega * y1"}, {{"omega", 2.0}});
    auto&& y = state{1.0};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 1);
    EXPECT_DOUBLE_EQ(dy[0], -4.0);
}

TEST(expr_rhs, combined_expression) {
    auto&& rhs = make_rhs({"y2 + sin(t) - alpha * y1", "0.0"}, {{"alpha", 0.5}});
    auto&& y = state{2.0, 3.0};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 2);
    EXPECT_DOUBLE_EQ(dy[0], 3.0 + 0.0 - 1.0);
}

TEST(expr_rhs, lorenz_rhs) {
    auto&& rhs = make_rhs(
        {
            "sigma * (y2 - y1)",
            "y1 * (rho - y3) - y2",
            "y1 * y2 - beta * y3",
        },
        {{"sigma", 10.0}, {"rho", 28.0}, {"beta", 8.0 / 3.0}});

    auto&& y = state{1.0, 1.0, 1.0};
    auto&& dy = rhs(0.0, y);

    ASSERT_EQ(dy.size(), 3);
    EXPECT_DOUBLE_EQ(dy[0], 0.0);
    EXPECT_DOUBLE_EQ(dy[1], 1.0 * (28.0 - 1.0) - 1.0);
    EXPECT_DOUBLE_EQ(dy[2], 1.0 - 8.0 / 3.0);
}

TEST(expr_rhs, rejects_empty_expression) {
    EXPECT_THROW(make_rhs({""}), invalid_problem_error);
}

TEST(expr_rhs, rejects_invalid_syntax) {
    EXPECT_THROW(make_rhs({"y1 +"}), invalid_problem_error);
}

TEST(expr_rhs, rejects_unknown_variable) {
    EXPECT_THROW(make_rhs({"y2"}), invalid_problem_error);
}

TEST(expr_rhs, throws_on_size_mismatch) {
    auto&& rhs = make_rhs({"y1"});
    auto&& y = state{1.0, 2.0};

    EXPECT_THROW(std::ignore = rhs(0.0, y), invalid_problem_error);
}

}  // namespace
