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
        }
        catch (const std::out_of_range&) {
            throw std::runtime_error("Settings key not found: " + key);
        }
        catch (const std::bad_any_cast&) {
            throw std::runtime_error("Settings type mismatch for key: " + key);
        }
    }

    template<typename T>
    T get_or(const std::string& key, const T& default_val) const {
        try {
            return std::any_cast<T>(data->at(key));
        }
        catch (...) {
            LogWarn("Settings key not found: '{}', using default value: '{}'", key, default_val);
            return default_val;
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