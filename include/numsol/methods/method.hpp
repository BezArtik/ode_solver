/**
 * @file method.hpp
 * @brief Set of available numerical methods.
 */

#pragma once

#include <array>
#include <string_view>
#include <variant>

#include "numsol/methods/euler.hpp"
#include "numsol/methods/rk2.hpp"
#include "numsol/methods/rk3.hpp"
#include "numsol/methods/rk4.hpp"

namespace numsol {

/**
 * @brief Variant holding any available numerical method.
 */
using method = std::variant<euler, rk2, rk3, rk4>;

/**
 * @brief Entry of @ref method_table.
 */
using method_entry = std::pair<std::string_view, method>;

/**
 * @brief Table of all available methods with their names.
 *
 * Used to look up a method by name at runtime.
 */
// clang-format off
inline constexpr std::array method_table = {
    method_entry{"euler", euler{}}, 
    method_entry{"rk2", rk2{}},
    method_entry{"rk3", rk3{}}, 
    method_entry{"rk4", rk4{}}
};
// clang-format on
}  // namespace numsol
