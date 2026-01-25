#pragma once
#include <map>
#include <string>
#include <memory>
#include <iostream>

namespace Izo {

template <typename T>
class ResourceManager {
public:
    virtual ~ResourceManager() {
        unload_all();
    }

    template <typename... Args>
    T* load(const std::string& name, Args&&... args) {
        auto res = std::make_shared<T>(std::forward<Args>(args)...);

        if (!res->valid()) {
            Logger::the().error(std::format("ResourceManager: Failed to load {0}", name));
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
