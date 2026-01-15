#include "File.hpp"
#include <fstream>
#include <sstream>

namespace Izo {

std::string File::read_all_text(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

} // namespace Izo
