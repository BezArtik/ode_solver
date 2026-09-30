#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace numsol::app {

struct parsed_rhs {
    std::vector<std::string> expressions_;
};

[[nodiscard]] parsed_rhs parse_rhs_expressions(const std::unordered_map<std::string, std::string>& exprs);

}  // namespace numsol::app
