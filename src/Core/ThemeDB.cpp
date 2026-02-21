#include "ThemeDB.hpp"
#include "Debug/Logger.hpp"
#include "Core/File.hpp"
#include "Graphics/ColorVariant.hpp"
#include "Graphics/Font.hpp"
#include "ViewManager.hpp"
#include <sstream>
#include <format>
#include <filesystem>
#include <algorithm>
#include "Core/ResourceManager.hpp"
#include "UI/Widgets/Widget.hpp"


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
    LogInfo("Theme Details:");
    LogInfo("  Name: {}", get<std::string>("ThemeManifest", "Name", "Unknown"));
    LogInfo("  Author: {}", get<std::string>("ThemeManifest", "Author", "Unknown"));
    LogInfo("  Version: {}", get<std::string>("ThemeManifest", "Version", "0.0.0"));

    auto fontFamily = ThemeDB::the().get<std::string>("System", "FontFamily", "fonts/Roboto-Regular.ttf");
    auto fontSize = ThemeDB::the().get<float>("System", "FontSize", 32.0);
    FontManager::the().reload("system-ui", fontFamily, fontSize);
    Widget::notify_theme_update_all();

    current_path = path;
    return true;
}

bool ThemeDB::reload() {
    if (current_path.empty()) return false;
    return load(current_path);
}

std::vector<std::string> ThemeDB::theme_names(const std::string& directory) const {
    std::filesystem::path root(ResourceManagerBase::resource_root());
    std::filesystem::path relative(directory);
    std::string full_path = (root / relative).string();

    if (!File::exists(full_path) || !File::is_directory(full_path)) {
        LogWarn("Theme directory not found: '{}'", full_path);
        return {};
    }

    std::vector<std::string> names;

    auto push_unique = [&](const std::string& name) {
        if (name.empty()) return;
        for (const auto& existing : names) {
            if (existing == name) return;
        }
        names.push_back(name);
    };

    auto entries = File::list_directory(full_path);
    for (const auto& entry : entries) {
        if (File::is_directory(entry)) continue;
        if (File::get_extension(entry) != ".ini") continue;
        std::string filename = File::get_filename(entry);
        size_t dot = filename.find_last_of('.');
        std::string name = (dot == std::string::npos) ? filename : filename.substr(0, dot);
        push_unique(name);
    }

    std::sort(names.begin(), names.end());

    return names;
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
