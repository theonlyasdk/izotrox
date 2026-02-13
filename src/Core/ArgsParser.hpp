#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Izo {
class ArgsParser {
   public:
    enum class ParseResult {
        ParseError,
        ParseOK,
        HelpRequested,
    };

    ArgsParser(const std::string& description = "");

    void add_argument(const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);
    void add_argument(const std::string& name, const std::string& short_name,
                      const std::string& description, bool required,
                      std::function<std::expected<bool, std::string>(std::string)> apply_value);
    void add_argument(std::string& value, const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);
    void add_argument(bool& value, const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);

    void add_flag(const std::string& name, const std::string& short_name,
                  const std::string& description);

    template <typename T>
    void add_argument(const std::string& name, const std::string& short_name,
                      const std::string& description, bool required = false);

    ParseResult parse(int argc, const char* argv[]);

    std::optional<std::string> value(const std::string& name) const;

    template <typename T>
    std::optional<T> typed_value(const std::string& name) const;

    template <typename T>
    std::vector<T> vector(const std::string& name) const;

    bool has_flag(const std::string& name) const;
    std::vector<std::string> positional_values() const;

    const std::string help_str() const;
    std::string last_error() const;

   private:
    struct Argument {
        std::string name;
        std::string short_name;
        std::string description;
        bool required;
        std::vector<std::string> values;
        std::function<std::expected<bool, std::string>(std::string)> accept_value;
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
    ParseResult m_parse_result;
};
}  // namespace Izo