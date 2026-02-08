#pragma once

#include <map>
#include <string>
#include <any>
#include <memory>
#include <vector>
#include "Debug/Logger.hpp"

namespace Izo {

class Settings {
public:
    Settings();
    
    template<typename T>
    void set(const std::string& key, const T& value) {
        data->emplace(key, std::any(value));
    }
    
    template<typename T>
    T get(const std::string& key) const {
        try {
            return std::any_cast<T>(data->at(key));
        } catch (...) {
            LogError("Trying to read settings: Key not found: {}", key);
        }
    }

    template<typename T>
    T get_or(const std::string& key, const T& defaultValue) const {
        try {
            return std::any_cast<T>(data->at(key));
        }
        catch (...) {
            LogError("Settings read failed for key: {}", key);
            return defaultValue;
        }
    }

    bool has(const std::string& key) const;
    
    void remove(const std::string& key);
    
    std::vector<std::string> keys() const;
    
    static void init();
    static Settings& the();

private:
    std::unique_ptr<std::map<std::string, std::any>> data;
    static std::shared_ptr<Settings> instance;
};

} // namespace Izo