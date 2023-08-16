#pragma once

#include <string>

struct Sound
{
    Sound() = default;
    Sound(const char *path) : path(path) {}
    operator const char *() { return path.c_str(); }
    operator std::string &() { return path; }
    std::string path;
};