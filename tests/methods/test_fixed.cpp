#include <gtest/gtest.h>

#include "common/methods.hpp"
#include "numsol/methods/method.hpp"

namespace {

using namespace numsol;
using namespace numsol::tests;

TEST(fixed, euler_order_1) {
    expect_order<euler>(2.0, 0.5);
}

TEST(fixed, rk2_order_2) {
    expect_order<rk2>(4.0, 0.5);
}

TEST(fixed, rk3_order_3) {
    expect_order<rk3>(8.0, 1.0);
}

TEST(fixed, rk4_order_4) {
    expect_order<rk4>(16.0, 2.0);
}

TEST(fixed, rk4_abs_error_small) {
    expect_error_below<rk4>({.h0_ = 0.01}, 1e-7);
}

TEST(fixed, rk4_reaches_t_end) {
    expect_success<rk4>({.h0_ = 0.01});
}

}  // namespace
