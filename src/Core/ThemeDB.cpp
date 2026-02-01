#include "ThemeDB.hpp"
#include "Debug/Logger.hpp"
#include "File.hpp"
#include "Graphics/ColorVariant.hpp"
#include "ViewManager.hpp"
#include <sstream>
#include <format>
#include <filesystem>
#include "ResourceManager.hpp"


namespace Izo {

ThemeDB& ThemeDB::the() {
    static ThemeDB m_instance;
    return m_instance;
}

static void list_theme_sections_and_values(ini::IniFile& ini_file) {
    LogDebug("File has {} sections", ini_file.size());

    for (const auto& sectionPair : ini_file) {
        const std::string& sectionName = sectionPair.first;
        const ini::IniSection& section = sectionPair.second;

        LogDebug("Section '{}' has {} fields", sectionName, section.size());

        for (const auto& fieldPair : section) {
            const std::string& fieldName = fieldPair.first;
            const ini::IniField& field = fieldPair.second;

            LogDebug("  Field '{}' Value '{}'", fieldName, field.as<std::string>());
        }
    }
}


bool ThemeDB::load(const std::string& path) {
    std::filesystem::path root(ResourceManagerBase::resource_root());
    std::filesystem::path relative(path);
    std::string full_path = (root / relative).string();

    std::string content = File::read_all_text(full_path);

    if (content.empty()) {
        LogError("Failed to load theme from '{}': Empty file!", full_path);
        return false;
    }

    ini_file.decode(content);

    list_theme_sections_and_values(ini_file);

    LogInfo("Successfully loaded theme from {}!", path);
    current_path = path;
    return true;
}

bool ThemeDB::reload() {
    if (current_path.empty()) return false;
    return load(current_path);
}

Color ThemeDB::get_variant_color(ColorVariant variant) {
    std::string key = "Default";

    switch (variant) {
        case ColorVariant::Default: key = "Default"; break;
        case ColorVariant::Primary: key = "Primary"; break;
        case ColorVariant::Secondary: key = "Secondary"; break;
        case ColorVariant::Tertiary: key = "Tertiary"; break;
        case ColorVariant::Success: key = "Success"; break;
        case ColorVariant::Warning: key = "Warning"; break;
        case ColorVariant::Error: key = "Error"; break;
        case ColorVariant::Info: key = "Info"; break;
        case ColorVariant::Muted: key = "Muted"; break;
    }

    return get<Color>("ColorVariants", key, Color::White);
}

}
