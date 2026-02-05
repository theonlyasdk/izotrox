#include "Core/ArgsParser.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Debug/Logger.hpp"

namespace Izo {

ArgsParser::ArgsParser(const std::string& description)
    : m_description(description) {}

void ArgsParser::add_argument(const std::string& name, const std::string& short_name,
                              const std::string& description, bool required) {
    m_arguments.push_back({name, short_name, description, required, {}});
}

void ArgsParser::add_flag(const std::string& name, const std::string& short_name,
                          const std::string& description) {
    m_flags.push_back({name, short_name, description, false});
}

template<typename T>
void ArgsParser::add_argument(const std::string& name, const std::string& short_name,
                              const std::string& description, bool required) {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>);
    m_arguments.push_back({name, short_name, description, required, {}});
}

bool ArgsParser::parse(int argc, const char* argv[]) {
    m_error.clear();
    m_positional.clear();

    for (auto& f : m_flags) f.set = false;
    for (auto& a : m_arguments) a.values.clear();

    for (int i = 1; i < argc; ++i) {
        std::string token = argv[i];
        
        if (token == "--help" || token == "-h") {
            m_help_requested = true;
            return true;
        }
        
        bool matched = false;

        for (auto& f : m_flags) {
            if (token == "--" + f.name || token == "-" + f.short_name) {
                f.set = true;
                matched = true;
                break;
            }
        }
        if (matched) continue;

        for (auto& a : m_arguments) {
            if (token == "--" + a.name || token == "-" + a.short_name) {
                if (i + 1 >= argc) {
                    m_error = "Missing value for argument: " + a.name;
                    return false;
                }
                a.values.push_back(argv[++i]);
                matched = true;
                break;
            }
        }
        if (matched) continue;

        m_positional.push_back(token);
    }

    for (const auto& a : m_arguments) {
        if (a.required && a.values.empty()) {
            m_error = "Missing required argument: " + a.name;
            return false;
        }
    }

    return true;
}

std::optional<std::string> ArgsParser::value(const std::string& name) const {
    for (const auto& a : m_arguments) {
        if (a.name == name && !a.values.empty()) {
            return a.values.front();
        }
    }
    return std::nullopt;
}

template<typename T>
std::optional<T> ArgsParser::typed_value(const std::string& name) const {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>);
    for (const auto& a : m_arguments) {
        if (a.name == name && !a.values.empty()) {
            if constexpr (std::is_same_v<T, std::string>) {
                return a.values.front();
            } else {
                std::istringstream iss(a.values.front());
                T out;
                if (iss >> out) return out;
            }
        }
    }
    return std::nullopt;
}

template<typename T>
std::vector<T> ArgsParser::vector(const std::string& name) const {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::string>);
    std::vector<T> result;
    for (const auto& a : m_arguments) {
        if (a.name == name) {
            for (const auto& v : a.values) {
                if constexpr (std::is_same_v<T, std::string>) {
                    result.push_back(v);
                } else {
                    std::istringstream iss(v);
                    T out;
                    if (iss >> out) result.push_back(out);
                }
            }
        }
    }
    return result;
}

bool ArgsParser::has_flag(const std::string& name) const {
    for (const auto& f : m_flags) {
        if (f.name == name) return f.set;
    }
    return false;
}

std::vector<std::string> ArgsParser::positional_values() const {
    return m_positional;
}

const std::string ArgsParser::help_str() const {
    std::stringstream help_string;
    help_string << m_description << "\n";
    for (const auto& a : m_arguments) {
        help_string << "  --" << a.name << ", -" << a.short_name << " : " << a.description << "\n";
    }
    for (const auto& f : m_flags) {
        help_string << "  --" << f.name << ", -" << f.short_name << " : " << f.description << "\n";
    }
    return help_string.str();
}

std::string ArgsParser::get_error() const {
    return m_error;
}

bool ArgsParser::help_requested() const {
    return m_help_requested;
}

template void ArgsParser::add_argument<int>(const std::string&, const std::string&, const std::string&, bool);
template void ArgsParser::add_argument<float>(const std::string&, const std::string&, const std::string&, bool);
template void ArgsParser::add_argument<std::string>(const std::string&, const std::string&, const std::string&, bool);

template std::optional<int> ArgsParser::typed_value<int>(const std::string&) const;
template std::optional<float> ArgsParser::typed_value<float>(const std::string&) const;
template std::optional<std::string> ArgsParser::typed_value<std::string>(const std::string&) const;

template std::vector<int> ArgsParser::vector<int>(const std::string&) const;
template std::vector<float> ArgsParser::vector<float>(const std::string&) const;
template std::vector<std::string> ArgsParser::vector<std::string>(const std::string&) const;

} 
