#include "numsol/io/solution_writer.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <string>

#include "numsol/core/errors.hpp"

namespace numsol::app {

void write_solution_csv(const solution& sol, std::ostream& out) {
    auto&& t = sol.t_;
    auto&& y = sol.y_;

    if (t.empty() || y.empty()) return;

    auto&& dimension = y.front().size();

    out << std::setprecision(std::numeric_limits<scalar>::max_digits10);
    out << 't';
    for (std::size_t i = 1; i <= dimension; ++i) out << ",y" << i;
    out << '\n';

    for (std::size_t k = 0; k < t.size(); ++k) {
        out << t[k];
        for (auto&& value : y[k]) out << ',' << value;
        out << '\n';
    }
}

void write_solution_csv(const solution& sol, const std::string& path) {
    auto&& file = std::ofstream{path};
    if (!file.is_open()) {
        throw invalid_problem_error{std::format("failed to open file: '{}'", path)};
    }
    write_solution_csv(sol, file);
}

}  // namespace numsol::app
