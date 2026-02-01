#include "IzoShell.hpp"
#include "ThemeDB.hpp"
#include "Application.hpp"
#include "Debug/Logger.hpp"
#include "File.hpp"
#include "Graphics/Toast.hpp"
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace Izo {

IzoShell& IzoShell::the() {
    static IzoShell instance;
    return instance;
}

IzoShell::IzoShell() {
    register_command("help", "Display available commands", "help [command]",
        [this](const std::vector<std::string>& args) {
            if (args.size() > 1) {
                auto it = m_commands.find(args[1]);
                if (it != m_commands.end()) {
                    Logger::the().info("Command: " + it->second.name);
                    Logger::the().info("Description: " + it->second.description);
                    Logger::the().info("Usage: " + it->second.usage);
                } else {
                    Logger::the().error("Unknown command: " + args[1]);
                }
            } else {
                Logger::the().info("\n" + help());
            }
        });

    register_command("version", "Display Izotrox version", "version",
        [](const std::vector<std::string>&) {
            Logger::the().info("Izotrox - Experimental GUI Framework for Android and Linux - (c) theonlyasdk 2026");
        });

    register_command("theme", "Theme management", "theme <load|list|reload> [name]",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                Logger::the().error("Usage: theme <load|list|reload> [name]");
                return;
            }

            std::string subcmd = args[1];
            std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);

            if (subcmd == "load") {
                if (args.size() < 3) {
                    Logger::the().error("Usage: theme load <name>");
                    return;
                }
                std::string theme_name = args[2];
                std::string path = "res/theme/" + theme_name + ".ini";

                if (ThemeDB::the().load(path)) {
                    ToastManager::the().show("Theme loaded: " + theme_name + " (" + path + ")");
                } else {
                    ToastManager::the().show("Failed to load theme: " + path);
                }
            } else if (subcmd == "list") {
                std::string theme_dir = "res/theme";
                try {
                    if (std::filesystem::exists(theme_dir) && std::filesystem::is_directory(theme_dir)) {
                        Logger::the().info("Available themes:");
                        for (const auto& entry : std::filesystem::directory_iterator(theme_dir)) {
                            if (entry.is_regular_file() && entry.path().extension() == ".ini") {
                                std::string filename = entry.path().filename().string();
                                std::string theme_name = filename.substr(0, filename.find_last_of('.'));
                                Logger::the().info("  " + theme_name + " (" + entry.path().string() + ")");
                            }
                        }
                    } else {
                        Logger::the().error("Theme directory not found: " + theme_dir);
                    }
                } catch (const std::exception& e) {
                    Logger::the().error("Error listing themes: " + std::string(e.what()));
                }
            } else if (subcmd == "reload") {
                if (ThemeDB::the().reload()) {
                    Logger::the().info("Theme reloaded successfully");
                } else {
                    Logger::the().error("Failed to reload theme");
                }
            } else {
                Logger::the().error("Unknown theme subcommand: " + subcmd);
                Logger::the().info("Usage: theme <load|list> [name]");
            }
        });

    register_command("fps", "Display current FPS", "fps",
        [](const std::vector<std::string>&) {
            float fps = 1000.0f / Application::the().delta();
            Logger::the().info("Current FPS: " + std::to_string((int)fps));
        });

    register_command("delta", "Display frame delta time", "delta",
        [](const std::vector<std::string>&) {
            float delta = Application::the().delta();
            Logger::the().info("Delta time: " + std::to_string(delta) + "ms");
        });

    register_command("getcolor", "Get color value for a tag", "getcolor <tag>",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                Logger::the().error("Usage: getcolor <tag>");
                return;
            }
            Color c = ThemeDB::the().get<Color>("Colors", args[1], Color(255));
            Logger::the().info(args[1] + " = rgba(" +
                std::to_string(c.r) + ", " +
                std::to_string(c.g) + ", " +
                std::to_string(c.b) + ", " +
                std::to_string(c.a) + ")");
        });

    register_command("exit", "Exit the application", "exit",
        [](const std::vector<std::string>&) {
            Logger::the().info("Exiting application...");
            Application::the().quit(0);
        });

    register_command("toast", "Show a toast message", "toast <message>",
        [](const std::vector<std::string>& args) {
            if (args.size() < 2) {
                Logger::the().error("Usage: toast <message>");
                return;
            }

            std::string message;
            for (size_t i = 1; i < args.size(); ++i) {
                message += args[i];
                if (i < args.size() - 1) message += " ";
            }

            Logger::the().info("Showing toast: " + message);
            ToastManager::the().show(message);
        });
}

void IzoShell::register_command(const std::string& name, const std::string& description,
                                const std::string& usage, CommandHandler handler) {
    m_commands[name] = {name, description, usage, handler};
}

void IzoShell::execute(const std::string& input) {
    if (input.empty()) return;

    auto args = split(input, ' ');
    if (args.empty()) return;

    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    auto it = m_commands.find(cmd);
    if (it != m_commands.end()) {
        try {
            it->second.handler(args);
        } catch (const std::exception& e) {
            Logger::the().error("Command execution failed: " + std::string(e.what()));
        }
    } else {
        Logger::the().error("Unknown command: " + cmd + ". Type 'help' for available commands.");
    }
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
