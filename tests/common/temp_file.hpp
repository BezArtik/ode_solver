#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace numsol::tests {

class temp_file_test : public ::testing::Test {
protected:
    std::filesystem::path path_;

    void SetUp() override {
        auto&& info = ::testing::UnitTest::GetInstance()->current_test_info();

        auto&& name = std::format("numsol_test_{}_{}_{}", info->test_suite_name(), info->name(), info->line());

        std::ranges::replace(name, '/', '_');

        path_ = std::filesystem::temp_directory_path() / name;
    }

    void TearDown() override {
        auto&& ec = std::error_code{};
        std::filesystem::remove(path_, ec);
    }

    void write(std::string_view content) {
        auto&& file = std::ofstream{path_};
        ASSERT_TRUE(file.is_open());
        file << content;
    }

    [[nodiscard]] std::string path_string() const { return path_.string(); }
};

}  // namespace numsol::tests
