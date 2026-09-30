#pragma once

#include <span>
#include <vector>

namespace numsol {

using scalar = double;
using time = scalar;
using state = std::vector<scalar>;
using state_view = std::span<const scalar>;

}  // namespace numsol
