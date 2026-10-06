#include <gtest/gtest.h>

#include <string>

#include "numsol/core/errors.hpp"
#include "numsol/expr/expr_parser.hpp"

namespace {

using namespace numsol;
using namespace numsol::app;

TEST(parse_rhs_expressions, orders_by_index) {
    auto&& exprs = expression_map{
        {"y2", "y1"},
        {"y1", "y2"},
        {"y3", "y1 + y2"},
    };

    auto&& parsed = parse_rhs_expressions(std::move(exprs));

    ASSERT_EQ(parsed.size(), 3);
    EXPECT_EQ(parsed[0], "y2");
    EXPECT_EQ(parsed[1], "y1");
    EXPECT_EQ(parsed[2], "y1 + y2");
}

TEST(parse_rhs_expressions, single_variable) {
    auto&& exprs = expression_map{
        {"y1", "-y1"},
    };

    auto&& parsed = parse_rhs_expressions(std::move(exprs));

    ASSERT_EQ(parsed.size(), 1u);
    EXPECT_EQ(parsed[0], "-y1");
}

TEST(parse_rhs_expressions, rejects_empty) {
    auto&& exprs = expression_map{};
    EXPECT_THROW(std::ignore = parse_rhs_expressions(std::move(exprs)), invalid_problem_error);
}

TEST(parse_rhs_expressions, rejects_non_contiguous) {
    auto&& exprs = expression_map{
        {"y1", "0"},
        {"y3", "0"},
    };
    EXPECT_THROW(std::ignore = parse_rhs_expressions(std::move(exprs)), invalid_problem_error);
}

TEST(parse_rhs_expressions, rejects_zero_index) {
    auto&& exprs = expression_map{
        {"y0", "0"},
    };
    EXPECT_THROW(std::ignore = parse_rhs_expressions(std::move(exprs)), invalid_problem_error);
}

TEST(parse_rhs_expressions, rejects_malformed_key) {
    auto&& exprs = expression_map{
        {"z1", "0"},
    };
    EXPECT_THROW(std::ignore = parse_rhs_expressions(std::move(exprs)), invalid_problem_error);
}

TEST(parse_rhs_expressions, rejects_non_numeric_suffix) {
    auto&& exprs = expression_map{
        {"y1a", "0"},
    };
    EXPECT_THROW(std::ignore = parse_rhs_expressions(std::move(exprs)), invalid_problem_error);
}

}  // namespace
