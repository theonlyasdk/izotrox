#pragma once

#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "Debug/Logger.hpp"

namespace Izo {

template <typename T>
class ResourceManager;

class ResourceManagerBase {
   public:
    static std::string resource_root();
    static std::string to_resource_path(const std::string& path);
    static void set_resource_root(const std::string& path);
    static bool is_valid_resource_dir(const std::string& path);

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
    T* load(const std::string& name, const std::string& path, Args&&... args) {
        std::filesystem::path root(resource_root());
        std::filesystem::path full_path = root / path;

        auto it = resources.find(name);
        if (it != resources.end())
            return it->second.get();

        auto res = std::make_unique<T>(full_path.string(), std::forward<Args>(args)...);

        if (!res || !res->valid()) {
            LogError("ResourceManager: Failed to load resource '{}' from '{}'", name, full_path.string());
            return {};
        }

        auto res_ptr = res.get();

        resources.emplace(name, std::move(res));

        return res_ptr;
    }

    /* If T is loaded, returns T*, otherwise loads it and returns T* */
    template <typename... Args>
    T* get_or_load(const std::string& name, const std::string& path, Args&&... args) {
        auto it = resources.find(name);
        if (it == resources.end()) {
            return load(name, path, std::forward<Args>(args)...);
        }

        return it->second.get();
    }

    /* If T is loaded, returns T*, otherwise nullopt */
    T* get(const std::string& name) const {
        auto it = resources.find(name);
        return (it != resources.end()) ? it->second.get() : nullptr;
    }

    T* get_or_crash(const std::string& name) const {
        auto it = resources.find(name);

        if (it == resources.end()) {
            LogFatal("Tried using unloaded resource: {}", name);
        }

        return it->second.get();
    }

    template <typename... Args>
    T* reload(const std::string& name, const std::string& path, Args&&... args)
    {
        resources.erase(name);
        return load(name, path, std::forward<Args>(args)...);
    }

    T* reload_from_existing(const std::string& name, std::unique_ptr<T> res)
    {
        if (!res || !res->valid()) {
            LogError("Attempted to replace '{}' with an invalid resource", name);
            return nullptr;
        }
        resources.erase(name);
        T* ptr = res.get();
        resources.emplace(name, std::move(res));
        return ptr;
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

    std::map<std::string, std::unique_ptr<T>> resources;
};

}  // namespace Izo
