#pragma once

#include <mutex>
#include <string>
#include <memory>
#include <format>
#include "Core/Application.hpp"

/* If LogFatal() should terminate the app with LOG_FATAL_EXIT_CODE */
#define LOG_FATAL_TERMINATES_APP
#ifdef LOG_FATAL_TERMINATES_APP
#define LOG_FATAL_EXIT_CODE 1
#endif

#define LOG_DEBUG_OUTPUT

namespace Izo {

enum class LogLevel {
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

    void enable_logging_to_file();
    void log(LogLevel lvl, const std::string& msg);

    // Template functions that handle std::format internally
    template<typename... Args>
    void trace(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Trace, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Warn, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void fatal(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Fatal, std::format(fmt, std::forward<Args>(args)...));
        #ifdef LOG_FATAL_TERMINATES_APP
            Application::the().quit(LOG_FATAL_EXIT_CODE);
        #endif
    }

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    struct LogFile;
    std::mutex               m_log_mutex;
    std::unique_ptr<LogFile> m_log_file;
};

/* Macro definitions for different log levels */
#define LogTrace(fmt, ...) \
    Logger::the().trace(fmt, ##__VA_ARGS__)

#ifdef LOG_DEBUG_OUTPUT
#define LogDebug(fmt, ...) \
    Logger::the().debug(fmt, ##__VA_ARGS__)
#else
#define LogDebug(fmt, ...)
#endif

#define LogInfo(fmt, ...) \
    Logger::the().info(fmt, ##__VA_ARGS__)

#define LogWarn(fmt, ...) \
    Logger::the().warn(fmt, ##__VA_ARGS__)

#define LogError(fmt, ...) \
    Logger::the().error(fmt, ##__VA_ARGS__)

#define LogFatal(fmt, ...) \
    Logger::the().fatal("At {}:{}: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

}
