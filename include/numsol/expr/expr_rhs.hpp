#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "numsol/core/types.hpp"

namespace numsol::app {

class expr_rhs {
public:
    expr_rhs(std::vector<std::string> expressions, std::unordered_map<std::string, scalar> params = {});
    ~expr_rhs();

    expr_rhs(expr_rhs&&) noexcept;
    expr_rhs& operator=(expr_rhs&&) noexcept;

    expr_rhs(const expr_rhs&) = delete;
    expr_rhs& operator=(const expr_rhs&) = delete;

    [[nodiscard]] state operator()(time t, state_view y) const;

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

}  // namespace numsol::app
