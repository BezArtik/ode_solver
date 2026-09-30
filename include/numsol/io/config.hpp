#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "numsol/core/options.hpp"
#include "numsol/core/types.hpp"

namespace numsol::app {

struct config {
    state y0_;
    time t0_ = 0.0;
    time t_end_ = 0.0;
    std::vector<std::string> rhs_expressions_;
    std::unordered_map<std::string, scalar> params_;

    std::string method_ = "rk4";
    solver_options opts_;

    std::string output_path_;
    std::string output_format_ = "csv";
};

[[nodiscard]] config load_config(std::string_view path);

}  // namespace numsol::app
