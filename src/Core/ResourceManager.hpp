#pragma once

#include <map>
#include <string>
#include <memory>
#include <format>
#include <filesystem>

#include "Debug/Logger.hpp"


namespace Izo {

template <typename T>
class ResourceManager;

class ResourceManagerBase {
public:
    static std::string resource_root();
    static std::string to_resource_path(const std::string& path);
    static void set_resource_root(const std::string& path);
private:
    static std::string s_resource_root;
};

template <typename T>
class ResourceManager : public ResourceManagerBase {
public:
    static ResourceManager& the() {
        static ResourceManager instance;
        return instance;
    }

    ~ResourceManager() = default;

    template <typename... Args>
    std::shared_ptr<T> load(const std::string& name,
                            const std::string& path,
                            Args&&... args)
    {
        std::filesystem::path root(resource_root());
        std::filesystem::path full = root / path;

        auto it = resources.find(name);
        if (it != resources.end())
            return it->second;

        auto res = std::make_shared<T>(full.string(),
                                       std::forward<Args>(args)...);

        if (!res || !res->valid()) {
            LogError("ResourceManager: Failed to load resource '{}' from '{}'",
                     name, full.string());
            return {};
        }

        resources.emplace(name, res);
        return res;
    }

    T* get(const std::string& name) const {
        auto it = resources.find(name);
        return (it != resources.end()) ? it->second.get() : nullptr;
    }

    void unload(const std::string& name) {
        resources.erase(name);
    }

    void unload_all() {
        resources.clear();
    }

private:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::map<std::string, std::shared_ptr<T>> resources;
};

}
