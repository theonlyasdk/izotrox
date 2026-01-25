#pragma once
#include <string>
#include <vector>
#include <optional>

namespace Izo {

class File {
public:
    static std::string read_all_text(const std::string& path);
};

} 