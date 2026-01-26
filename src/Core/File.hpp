#pragma once

#include <string>

namespace Izo {

class File {
public:
    static std::string read_all_text(const std::string& path);
};

} 