#pragma once

#include <array>
#include <string_view>
#include <variant>

#include "numsol/methods/euler.hpp"
#include "numsol/methods/rk2.hpp"
#include "numsol/methods/rk3.hpp"
#include "numsol/methods/rk4.hpp"

namespace numsol {

using method = std::variant<euler, rk2, rk3, rk4>;

using method_entry = std::pair<std::string_view, method>;
// clang-format off
inline constexpr std::array method_table = {
    method_entry{"euler", euler{}}, 
    method_entry{"rk2", rk2{}},
    method_entry{"rk3", rk3{}}, 
    method_entry{"rk4", rk4{}}
};
// clang-format on
}  // namespace numsol
