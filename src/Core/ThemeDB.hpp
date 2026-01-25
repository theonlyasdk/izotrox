#pragma once

#include "Graphics/Color.hpp"
#include "Graphics/ColorVariant.hpp"
#include "Graphics/Animator.hpp"
#include <map>
#include <string>
#include <vector>

namespace Izo {

class ThemeDB {
public:
    static ThemeDB& the();

    bool load(const std::string& path);
    bool reload();
    std::vector<std::string> list_tags() const;

    Color color(const std::string& name);
    Color variant_color(ColorVariant variant);

    int int_value(const std::string& name, int defaultVal = 0);
    std::string string_value(const std::string& name, const std::string& defaultVal = "");

    template<typename T>
    T enum_value(const std::string& name, T defaultVal) {
        return defaultVal;
    }

private:
    std::string current_path;
    std::map<std::string, Color> colors;
    std::map<std::string, int> values;
    std::map<std::string, std::string> strings;

    std::string trim(const std::string& s);
    Color parse_color(const std::string& s);
};

} 
