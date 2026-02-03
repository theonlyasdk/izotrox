#pragma once

#include <map>
#include <string>
#include <any>
#include <memory>
#include <vector>

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
        return std::any_cast<T>(data->at(key));
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