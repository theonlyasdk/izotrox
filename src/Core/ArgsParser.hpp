#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <type_traits>

namespace Izo {
class ArgsParser {
public:
    ArgsParser(const std::string& description = "");

    void add_argument(const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);

    void add_flag(const std::string& name, const std::string& short_name,
                  const std::string& description);

    template<typename T>
    void add_argument(const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);

    bool parse(int argc, const char* argv[]);

    std::optional<std::string> value(const std::string& name) const;

    template<typename T>
    std::optional<T> typed_value(const std::string& name) const;

    template<typename T>
    std::vector<T> vector(const std::string& name) const;

    bool has_flag(const std::string& name) const;
    std::vector<std::string> positional_values() const;
    
    bool help_requested() const;
    const std::string help_str() const;
    std::string get_error() const;

private:
    struct Argument {
        std::string name;
        std::string short_name;
        std::string description;
        bool required;
        std::vector<std::string> values;
    };

    struct Flag {
        std::string name;
        std::string short_name;
        std::string description;
        bool set;
    };

    std::string m_description;
    std::vector<Argument> m_arguments;
    std::vector<Flag> m_flags;
    std::vector<std::string> m_positional;
    std::string m_error;
    bool m_help_requested = false;
};
}