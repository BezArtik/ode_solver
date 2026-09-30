#pragma once

#include "numsol/core/types.hpp"

namespace numsol {

struct step_result {
    state y_next_;
    time t_next_ = 0.0;
};

}  // namespace numsol
