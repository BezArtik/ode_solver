#pragma once

#include <stdexcept>

namespace numsol {

struct solver_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct invalid_problem_error : solver_error {
    using solver_error::solver_error;
};

struct max_steps_exceeded_error : solver_error {
    using solver_error::solver_error;
};

struct step_size_too_small_error : solver_error {
    using solver_error::solver_error;
};

struct not_implemented_error : solver_error {
    using solver_error::solver_error;
};

}  // namespace numsol
