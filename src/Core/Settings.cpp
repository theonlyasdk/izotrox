#include "Settings.hpp"

namespace Izo {

std::shared_ptr<Settings> Settings::instance = nullptr;

Settings::Settings() 
    : data(std::make_unique<std::map<std::string, std::any>>()) {
}

bool Settings::has(const std::string& key) const {
    return data->find(key) != data->end();
}

void Settings::remove(const std::string& key) {
    data->erase(key);
}

std::vector<std::string> Settings::keys() const {
    std::vector<std::string> result;
    for (const auto& pair : *data) {
        result.push_back(pair.first);
    }
    return result;
}

void Settings::init() {
    if (!instance) {
        instance = std::make_shared<Settings>();
    }
}

Settings& Settings::the() {
    if (!instance) {
        init();
    }
    return *instance;
}

} // namespace Izo