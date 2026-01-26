#pragma once

#include <mutex>
#include <string>
#include <memory>

namespace Izo {

enum class Level {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    static Logger& the();

    void set_level(Level lvl);
    void enable_logging_to_file();
    void log(Level lvl, const std::string& msg);

    void trace(const std::string& msg);
    void debug(const std::string& msg);
    void info (const std::string& msg);
    void warn (const std::string& msg);
    void error(const std::string& msg);
    void fatal(const std::string& msg);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string format(Level lvl, const std::string& msg);
    std::string format_plain(Level lvl, const std::string& msg);
    std::string timestamp();
    std::string level_to_string(Level lvl);

    Level               min_level_;
    std::mutex          mutex_;
    struct LogFile;
    std::unique_ptr<LogFile> m_log_file;
};

} 
