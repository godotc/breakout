#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <log.h>
#include <sstream>

TEST(TestLoadLevel, should_read_the_right_level_from_plain_text)
{
    auto          file = "../res/levels/0_standard";
    std::ifstream ifs(file);
    if (ifs.fail() || !ifs.is_open()) {
        LOG_ERROR("load level file {} failed", file);
        return;
    }

    using ui = unsigned int;

    ui          tile_code;
    std::string line;

    using std::vector;
    vector<vector<ui>> tile_data;

    while (std::getline(ifs, line))
    {
        std::vector<ui> row;
        row.reserve(5);

        std::istringstream ss(line);

        // from string(raw hex) to unsigned int
        // and when reach a space will turn to next loop
        while (ss >> tile_code) {
            row.push_back(tile_code);
        }
        tile_data.push_back(row);
    }



    for (auto &row : tile_data) {
        for (auto c : row) {
            std::cout << c << ' ';
        }
        printf("\n");
    }
}
