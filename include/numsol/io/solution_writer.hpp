#pragma once

#include <iosfwd>
#include <string>

#include "numsol/core/solution.hpp"

namespace numsol::app {

void write_solution_csv(const solution& sol, std::ostream& out);
void write_solution_csv(const solution& sol, const std::string& path);

}  // namespace numsol::app
