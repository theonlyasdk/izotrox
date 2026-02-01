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
    virtual ~ResourceManager() {
        unload_all();
    }


    template <typename... Args>
    T* load(const std::string& name, const std::string& path, Args&&... args) {
        std::filesystem::path root(ResourceManagerBase::resource_root());
        std::filesystem::path relative(path);
        std::string full_path = (root / relative).string();

        auto res = std::make_shared<T>(full_path, std::forward<Args>(args)...);

        if (!res->valid()) {
            LogError("ResourceManager: Failed to load resource '{}' from '{}'", name, full_path);
            std::exit(1);
        }

        resources[name] = res;
        return res.get();
    }

    T* get(const std::string& name) {
        auto it = resources.find(name);
        if (it != resources.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void unload(const std::string& name) {
        resources.erase(name);
    }

    void unload_all() {
        resources.clear();
    }

private:
    std::map<std::string, std::shared_ptr<T>> resources;
};

}
