#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace Izo {

class IzoShell {
public:
    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;

    struct ExecuteResult {
        bool ok = true;
        std::string error;
        std::string output;
    };

    struct Command {
        std::string name;
        std::string description;
        std::string usage;
        CommandHandler handler;
    };

    static IzoShell& the();

    void register_all_commands();
    void register_command(const std::string& name, const std::string& description, 
                         const std::string& usage, CommandHandler handler);
    ExecuteResult execute(const std::string& input);
    std::string help() const;

private:
    IzoShell();

    std::map<std::string, Command> m_commands;
    std::vector<std::string> split(const std::string& str, char delimiter);
};

}
