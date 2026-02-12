#pragma once

#include "Debug/Logger.hpp"
#include "Debug/Generics.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/ColorVariant.hpp"
#include "Lib/inicpp.hpp"
#include "Lib/magic_enum.hpp"
#include <string>
#include <vector>

namespace Izo {

class ThemeDB {
public:
    static ThemeDB& the();

    bool load(const std::string& path);
    bool reload();
    Color get_variant_color(ColorVariant variant);
    std::vector<std::string> theme_names(const std::string& directory = "themes") const;

    template <typename T>
    T get(const std::string& section, const std::string& name, T default_val) const
    {
        auto fail = [&](const std::string& reason) -> T {
            LogWarn("Failed to find theme key '{}/{}': {}", section, name, reason);
            return default_val;
        };

        auto section_it = ini_file.find(section);
        if (section_it == ini_file.end()) 
            return fail("Key not found!");

        auto key_it = section_it->second.find(name);
        if (key_it == section_it->second.end()) 
            return fail("Key not found!");

        try {
            return key_it->second.template as<T>();
        } catch (...) {
            return fail(std::format("Unable to parse key as {}!", type_name<T>()));
        }
    }


private:
    std::string current_path;
    ini::IniFile ini_file;
};

} 

/* INI Parser extensions for Izo::Color and enums */
namespace ini
{
    template<>
    struct Convert<Izo::Color>
    {
        void decode(const std::string& value, Izo::Color& result)
        {
            std::string raw = value;

            // Remove all whitespace
            raw.erase(std::remove_if(raw.begin(), raw.end(),
                                     [](unsigned char c){ return std::isspace(c); }),
                      raw.end());

            std::istringstream ss(raw);
            int r, g, b;
            char c1, c2;

            if (ss >> r >> c1 >> g >> c2 >> b &&
                c1 == ',' && c2 == ',' &&
                r >= 0 && r <= 255 &&
                g >= 0 && g <= 255 &&
                b >= 0 && b <= 255)
            {
                result = Izo::Color{
                    static_cast<uint8_t>(r),
                    static_cast<uint8_t>(g),
                    static_cast<uint8_t>(b)
                };
                return;
            }

            throw std::runtime_error("Invalid Color format: " + value);
        }

        void encode(const Izo::Color& value, std::string& result)
        {
            std::ostringstream ss;
            ss << static_cast<int>(value.r) << ","
               << static_cast<int>(value.g) << ","
               << static_cast<int>(value.b);
            result = ss.str();
        }
    };

    template<typename T>
    requires std::is_enum_v<T>
    struct Convert<T>
    {
        void decode(const std::string& value, T& result)
        {
            if (auto opt = magic_enum::enum_cast<T>(value)) {
                result = *opt;
                return;
            }
            throw std::runtime_error("Invalid enum value: " + value);
        }

        void encode(const T& value, std::string& result)
        {
            result = std::string(magic_enum::enum_name(value));
        }
    };
}
