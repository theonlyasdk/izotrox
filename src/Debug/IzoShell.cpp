#include "IzoShell.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/Application.hpp"
#include "Debug/Logger.hpp"
#include "UI/Widgets/Toast.hpp"
#include "Core/ResourceManager.hpp"
#include "Core/File.hpp"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace Izo {

IzoShell& IzoShell::the() {
    static IzoShell instance;
    return instance;
}

IzoShell::IzoShell() {
    register_all_commands();
}

void IzoShell::register_all_commands() {
    register_command("help", "Display available commands", "help [command]",
        [this](const std::vector<std::string>& args) {
            if (args.size() > 1) {
                auto it = m_commands.find(args[1]);
                if (it != m_commands.end()) {
                    LogInfo("Command: {}", it->second.name);
                    LogInfo("Description: {}", it->second.description);
                    LogInfo("Usage: {}", it->second.usage);
                } else {
                    throw std::runtime_error("Unknown command: " + args[1]);
                }
            } else {
                LogInfo("\n{}", help());
            }
        });

    register_command("version", "Display Izotrox version", "version",
        [](const std::vector<std::string>&) {
            LogInfo("Izotrox - Experimental GUI Framework for Android and Linux - (c) theonlyasdk 2026");
        });

    register_command("theme", "Theme management", "theme <load|list|reload> [name]",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                throw std::runtime_error("Usage: theme <load|list|reload> [name]");
            }

            std::string subcmd = args[1];
            std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);

            if (subcmd == "load") {
                if (args.size() < 3) {
                    throw std::runtime_error("Usage: theme load <name>");
                }
                std::string theme_name = args[2];
                std::string path = "theme/" + theme_name + ".ini";

                if (ThemeDB::the().load(path)) {
                    ToastManager::the().show("Theme loaded: " + theme_name + " (" + path + ")");
                } else {
                    throw std::runtime_error("Failed to load theme: " + path);
                }
            } else if (subcmd == "list") {
                std::string theme_dir = ResourceManagerBase::to_resource_path("theme");
                try {
                    if (File::exists(theme_dir) && File::is_directory(theme_dir)) {
                        LogInfo("Available themes:");
                        for (const auto& entry : std::filesystem::directory_iterator(theme_dir)) {
                            if (entry.is_regular_file() && entry.path().extension() == ".ini") {
                                std::string filename = entry.path().filename().string();
                                std::string theme_name = filename.substr(0, filename.find_last_of('.'));
                                LogInfo("  {} ({})", theme_name, entry.path().string());
                            }
                        }
                    } else {
                        throw std::runtime_error("Theme directory not found: " + theme_dir);
                    }
                } catch (const std::exception& e) {
                    throw std::runtime_error("Error listing themes: " + std::string(e.what()));
                }
            } else if (subcmd == "reload") {
                if (ThemeDB::the().reload()) {
                    LogInfo("Theme reloaded successfully");
                } else {
                    throw std::runtime_error("Failed to reload theme");
                }
            } else {
                throw std::runtime_error("Unknown theme subcommand: " + subcmd);
            }
        });

    register_command("fps", "Display current FPS", "fps",
        [](const std::vector<std::string>&) {
            float fps = 1000.0f / Application::the().delta();
            LogInfo("Current FPS: {}", (int)fps);
        });

    register_command("delta", "Display frame delta time", "delta",
        [](const std::vector<std::string>&) {
            float delta = Application::the().delta();
            LogInfo("Delta time: {} ms", delta);
        });

    register_command("getcolor", "Get color value for a tag", "getcolor <tag>",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                throw std::runtime_error("Usage: getcolor <tag>");
            }
            Color c = ThemeDB::the().get<Color>("Colors", args[1], Color(255));
            LogInfo("{} = rgba({}, {}, {}, {})", args[1], c.r, c.g, c.b, c.a);
        });

    register_command("exit", "Exit the application", "exit",
        [](const std::vector<std::string>&) {
            LogInfo("Exiting application...");
            Application::the().quit(0);
        });

    register_command("toast", "Show a toast message", "toast <message>",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                throw std::runtime_error("Usage: toast <message>");
            }

            std::string message;
            for (size_t i = 1; i < args.size(); ++i) {
                message += args[i];
                if (i < args.size() - 1) message += " ";
            }

            LogInfo("Showing toast: {}", message);
            ToastManager::the().show(message);
        });

    register_command("debug", "Toggle debug mode", "debug <on|off>",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                throw std::runtime_error("Usage: debug <on|off>");
            }

            std::string subcmd = args[1];
            std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);

            if (subcmd == "on") {
                Application::the().set_debug(true);
                LogInfo("Debug mode enabled");
                ToastManager::the().show("Debug mode enabled");
            } else if (subcmd == "off") {
                Application::the().set_debug(false);
                LogInfo("Debug mode disabled");
                ToastManager::the().show("Debug mode disabled");
            } else {
                throw std::runtime_error("Usage: debug <on|off>");
            }
        });
}

void IzoShell::register_command(const std::string& name, const std::string& description,
                                const std::string& usage, CommandHandler handler) {
    m_commands[name] = {name, description, usage, handler};
}

IzoShell::ExecuteResult IzoShell::execute(const std::string& input) {
    if (input.empty()) {
        return {false, "Command cannot be empty"};
    }

    auto args = split(input, ' ');
    if (args.empty()) {
        return {false, "Command cannot be empty"};
    }

    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    auto it = m_commands.find(cmd);
    if (it != m_commands.end()) {
        try {
            it->second.handler(args);
            return {true, ""};
        } catch (const std::exception& e) {
            std::string error = std::string(e.what());
            LogError("Command execution failed: {}", error);
            return {false, error};
        }
    } else {
        std::string error = "Unknown command: " + cmd;
        LogError("{}", error);
        return {false, error};
    }

    return {true, ""};
}

std::string IzoShell::help() const {
    std::stringstream ss;
    ss << "Available commands:\n";
    for (const auto& pair : m_commands) {
        ss << "  " << pair.second.name << " - " << pair.second.description << "\n";
    }
    return ss.str();
}

std::vector<std::string> IzoShell::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

}
