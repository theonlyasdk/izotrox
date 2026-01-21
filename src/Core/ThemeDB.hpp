#pragma once

#include "../Graphics/Color.hpp"
#include "Debug/Logger.hpp"
#include "File.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>

namespace Izo {

class ThemeDB {
public:
    static ThemeDB& the() {
        static ThemeDB m_instance;
        return m_instance;
    }

    void load(const std::string& path) {
        std::string content = File::read_all_text(path);
        std::stringstream ss(content);
        std::string line;
        std::string section;

        while (std::getline(ss, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            
            if (line[0] == '[') {
                size_t end = line.find(']');
                if (end != std::string::npos) {
                    section = line.substr(1, end - 1);
                }
                continue;
            }

            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = trim(line.substr(0, eq));
                std::string val = trim(line.substr(eq + 1));
                
                if (section == "Colors") {
                    colors[key] = parse_color(val);
                } else if (section == "Values") {
                    try {
                        values[key] = std::stoi(val);
                    } catch (...) {}
                }
            }
        }

        Logger::the().info(std::format("Theme from '{}' has been loaded.", path));
    }

    Color color(const std::string& name) {
        auto it = colors.find(name);
        if (it != colors.end()) return it->second;
        return Color(128, 128, 128); 
    }

    int value(const std::string& name, int defaultVal = 0) {
        auto it = values.find(name);
        if (it != values.end()) return it->second;
        return defaultVal;
    }

private:
    std::map<std::string, Color> colors;
    std::map<std::string, int> values;

    std::string trim(const std::string& s) {
        size_t first = s.find_first_not_of(" 	\r\n");
        if (std::string::npos == first) return "";
        size_t last = s.find_last_not_of(" 	\r\n");
        return s.substr(first, (last - first + 1));
    }

    Color parse_color(const std::string& s) {
        std::stringstream ss(s);
        std::string segment;
        std::vector<int> vals;
        while (std::getline(ss, segment, ',')) {
            vals.push_back(std::stoi(trim(segment)));
        }
        if (vals.size() >= 4) return Color((uint8_t)vals[0], (uint8_t)vals[1], (uint8_t)vals[2], (uint8_t)vals[3]);
        if (vals.size() >= 3) return Color((uint8_t)vals[0], (uint8_t)vals[1], (uint8_t)vals[2]);
        return Color(0, 0, 0);
    }
};

} // namespace Izo