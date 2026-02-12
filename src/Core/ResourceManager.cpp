#include "Core/ResourceManager.hpp"
#include "Core/File.hpp"
#include "Debug/Logger.hpp"

namespace Izo {

std::string ResourceManagerBase::s_resource_root = "res/";

std::string ResourceManagerBase::resource_root() {
    return s_resource_root;
}

std::string ResourceManagerBase::to_resource_path(const std::string& path) {
    return s_resource_root + path;
}

void ResourceManagerBase::set_resource_root(const std::string& path) {
    s_resource_root = path;
    if (!s_resource_root.empty() && s_resource_root.back() != '/') {
        s_resource_root += '/';
    }
}

bool ResourceManagerBase::is_valid_resource_dir(const std::string &path) {
    // We check if the resource directory contains the following files
    // fonts, icons, theme
    std::vector<std::string> res_directories = {
        "fonts",
        "icons",
        "themes",
    };

    for (auto& dir : res_directories) {
        if (!File::is_directory(dir)) {
            return false;
        }
    }

    return true;
}

}
