#include "log.h"
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <optional>
#include <ostream>

#if _WIN32
#include <windows.h>
#endif

using namespace std;


TEST(TestPath, should_print_the_path)
{

    char path[1024]{};

    getcwd(path, 256);
    std::cout << "getcwd:\t ";
    std::cout << path << "\n";

    std::cout << "pwd:\t ";
    std::flush(std::cout);
    system("pwd");



#if _WIN32
    auto len = GetModuleFileName(nullptr, path, 1024);
    LOG_DEBUG("LEN == {}", len);
    if (len > 0 && len < 1024) {
        std::cout << "GetModuleFileName:\t Exe path:" << path << '\n';
    }
#else
    readlink("/proc/self/exe", path, 256);
    std::cout << "readlink:\t ";
    std::cout << path << "\n";
#endif

    std::cout << "filesystem:\t ";
    std::cout << std::filesystem::current_path() << '\n';

#if !_WIN32
    std::cout << __FILE_NAME__ << '\n';
#endif
    std::cout << __FILE__ << '\n';
}
