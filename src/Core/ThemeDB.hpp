#pragma once

#include "Graphics/Color.hpp"
#include <map>
#include <string>
#include <vector>

namespace Izo {

class ThemeDB {
public:
    static ThemeDB& the();

    void load(const std::string& path);

    Color color(const std::string& name);

    int int_value(const std::string& name, int defaultVal = 0);
    std::string string_value(const std::string& name, const std::string& defaultVal = "");

private:
    std::map<std::string, Color> colors;
    std::map<std::string, int> values;
    std::map<std::string, std::string> strings;

    std::string trim(const std::string& s);
    Color parse_color(const std::string& s);
};

} // namespace Izo
