#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "common/temp_file.hpp"
#include "numsol/core/errors.hpp"
#include "numsol/core/solution.hpp"
#include "numsol/io/solution_writer.hpp"

namespace {

using namespace numsol;
using namespace numsol::app;
using namespace numsol::tests;

auto make_solution(std::vector<numsol::time> t, std::vector<state> y) {
    auto&& sol = solution{};
    sol.t_ = std::move(t);
    sol.y_ = std::move(y);
    return sol;
}

TEST(solution_writer, writes_header) {
    auto&& sol = make_solution({0.0, 1.0}, {{1.0, 2.0}, {3.0, 4.0}});

    auto&& out = std::ostringstream{};
    write_solution_csv(sol, out);

    auto&& text = out.str();
    EXPECT_EQ(text.substr(0, text.find('\n')), "t,y1,y2");
}

TEST(solution_writer, writes_values) {
    auto&& sol = make_solution({0.0, 0.5}, {{1.0, 2.0}, {3.0, 4.0}});

    auto&& out = std::ostringstream{};
    write_solution_csv(sol, out);

    std::string_view expected =
        "t,y1,y2\n"
        "0,1,2\n"
        "0.5,3,4\n";
    EXPECT_EQ(out.str(), expected);
}

TEST(solution_writer, single_component) {
    auto&& sol = make_solution({0.0}, {{42.0}});

    auto&& out = std::ostringstream{};
    write_solution_csv(sol, out);

    std::string_view expected =
        "t,y1\n"
        "0,42\n";
    EXPECT_EQ(out.str(), expected);
}

TEST(solution_writer, empty_solution_writes_nothing) {
    auto&& sol = make_solution({}, {});

    auto&& out = std::ostringstream{};
    write_solution_csv(sol, out);

    EXPECT_TRUE(out.str().empty());
}

TEST(solution_writer, full_precision) {
    auto&& sol = make_solution({0.0}, {{0.123456789012345}});

    auto&& out = std::ostringstream{};
    write_solution_csv(sol, out);

    auto&& text = out.str();
    auto&& pos = text.find("0.123456789012345");
    EXPECT_NE(pos, std::string::npos);
}

TEST_F(temp_file_test, writes_to_file) {
    auto&& sol = make_solution({0.0, 1.0}, {{1.0, 2.0}, {3.0, 4.0}});

    write_solution_csv(sol, path_string());

    auto&& in = std::ifstream{path_};
    ASSERT_TRUE(in.is_open());

    auto&& line = std::string{};
    ASSERT_TRUE(std::getline(in, line));
    EXPECT_EQ(line, "t,y1,y2");

    ASSERT_TRUE(std::getline(in, line));
    EXPECT_EQ(line, "0,1,2");

    ASSERT_TRUE(std::getline(in, line));
    EXPECT_EQ(line, "1,3,4");
}

TEST_F(temp_file_test, throws_on_invalid_path) {
    auto&& sol = make_solution({0.0}, {{1.0}});

    auto&& bad = std::filesystem::temp_directory_path() / "numsol_nonexistent_dir_12345" / "out.csv";

    EXPECT_THROW(write_solution_csv(sol, bad.string()), invalid_problem_error);
}

}  // namespace
