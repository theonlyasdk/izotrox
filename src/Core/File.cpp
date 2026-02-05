#include "Core/File.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Debug/Logger.hpp"

namespace Izo {

std::string File::read_all_text(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

bool File::exists(const std::string &path) {
    bool found = std::filesystem::exists(path);

    if (found)
        LogDebug("Checking if {} exists: Found!", path);
    else
        LogDebug("Checking if {} exists: Not Found!", path);

    return found;
}

bool File::is_directory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

}
