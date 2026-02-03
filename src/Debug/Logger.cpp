#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <Debug/Logger.hpp>
#include <Debug/ConsoleColor.hpp>

namespace Izo {

static const char *color_for_level(Level lvl) {
  switch (lvl) {
    case Level::Trace:  return ConsoleColor::BrightBlack;
    case Level::Debug:  return ConsoleColor::BrightBlue;
    case Level::Info:   return ConsoleColor::BrightGreen;
    case Level::Warn:   return ConsoleColor::BrightYellow;
    case Level::Error:  return ConsoleColor::BrightRed;
    case Level::Fatal:  return ConsoleColor::BrightMagenta;
    default:            return ConsoleColor::Reset;
  }
}

struct Logger::LogFile {
    std::ofstream stream;
};

Logger::Logger(){}
Logger::~Logger() = default;

Logger &Logger::the() {
  static Logger s_instance;
  return s_instance;
}

void Logger::enable_logging_to_file() {
    std::string filename_str;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (m_log_file) return;

        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "izotrox-" << std::put_time(std::localtime(&itt), "%Y%m%d-%H%M%S") << ".log";
        std::string filename = ss.str();

        namespace fs = std::filesystem;
        fs::path log_dir = "logs";
        if (!fs::exists(log_dir)) {
            fs::create_directory(log_dir);
        }

        fs::path log_path = log_dir / filename;
        filename_str = log_path.string();

        m_log_file = std::make_unique<LogFile>();
        m_log_file->stream.open(log_path, std::ios::out | std::ios::app);
    }

    if (!filename_str.empty()) {
        info(std::format("Logging to file {}", filename_str));
    }
}

void Logger::log(Level lvl, const std::string &msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::cout << format(lvl, msg) << std::endl;

  if (m_log_file && m_log_file->stream.good()) {
      m_log_file->stream << format_plain(lvl, msg) << std::endl;
  }
}

void Logger::trace(const std::string &msg) { log(Level::Trace, msg); }
void Logger::debug(const std::string &msg) { log(Level::Debug, msg); }
void Logger::info(const std::string &msg) { log(Level::Info, msg); }
void Logger::warn(const std::string &msg) { log(Level::Warn, msg); }
void Logger::error(const std::string &msg) { log(Level::Error, msg); }
void Logger::fatal(const std::string &msg) {
    log(Level::Fatal, msg);

#ifdef LOG_FATAL_TERMINATES_APP
        Application::the().quit(LOG_FATAL_EXIT_CODE);
#endif
}

std::string Logger::format(Level lvl, const std::string &msg) {
  std::ostringstream oss;
  oss << ConsoleColor::BrightCyan << "Izotrox> " << ConsoleColor::Reset << "["
      << timestamp() << "]"
      << "(" << level_to_string(lvl) << ") " << color_for_level(lvl) << msg
      << ConsoleColor::Reset;
  return oss.str();
}

std::string Logger::format_plain(Level lvl, const std::string &msg) {
  std::ostringstream oss;
  oss << "Izotrox> [" << timestamp() << "]"
      << "(" << level_to_string(lvl) << ") " << msg;
  return oss.str();
}

std::string Logger::timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto itt = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream ss;
  ss << std::put_time(std::localtime(&itt), "%Y-%m-%d %H:%M:%S") << '.'
     << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

std::string Logger::level_to_string(Level lvl) {
  switch (lvl) {
    case Level::Trace:
      return "TRACE";
    case Level::Debug:
      return "DEBUG";
    case Level::Info:
      return "INFO";
    case Level::Warn:
      return "WARN";
    case Level::Error:
      return "ERROR";
    case Level::Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

}
