#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "numsol/core/problem.hpp"
#include "numsol/core/step.hpp"
#include "numsol/core/types.hpp"

namespace numsol {

template <typename Tableau>
class explicit_rk {
public:
    static constexpr std::size_t stages = Tableau::b.size();

    [[nodiscard]] std::int32_t order() const noexcept { return Tableau::order; }

    template <rhs F>
    [[nodiscard]] step_result step(F&& f, time t, state_view y, time h) const {
        auto&& k = std::array<state, stages>{};

        for (std::size_t i = 0; i < stages; ++i) {
            auto&& y_stage = state{y.begin(), y.end()};

            for (std::size_t j = 0; j < i; ++j) {
                auto&& a_ij = Tableau::a[i][j];
                if (a_ij == scalar{0}) continue;
                auto&& k_j = k[j];
                for (std::size_t m = 0; m < y_stage.size(); ++m) {
                    y_stage[m] += h * a_ij * k_j[m];
                }
            }

            k[i] = f(t + Tableau::c[i] * h, y_stage);
        }

        auto&& y_next = state{y.begin(), y.end()};
        for (std::size_t i = 0; i < stages; ++i) {
            auto&& b_i = Tableau::b[i];
            if (b_i == scalar{0}) continue;
            auto&& k_i = k[i];
            for (std::size_t m = 0; m < y_next.size(); ++m) {
                y_next[m] += h * b_i * k_i[m];
            }
        }

        return {
            .y_next_ = std::move(y_next),
            .t_next_ = t + h,
        };
    }
};

}  // namespace numsol
