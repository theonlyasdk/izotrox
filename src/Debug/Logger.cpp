#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <mutex>

#include <Debug/Logger.hpp>
#include <Debug/ConsoleColor.hpp>

namespace Izo {

static const char *color_for_level(LogLevel lvl) {
  switch (lvl) {
    case LogLevel::Trace:  return ConsoleColor::BrightBlack;
    case LogLevel::Debug:  return ConsoleColor::BrightBlue;
    case LogLevel::Info:   return ConsoleColor::BrightGreen;
    case LogLevel::Warn:   return ConsoleColor::BrightYellow;
    case LogLevel::Error:  return ConsoleColor::BrightRed;
    case LogLevel::Fatal:  return ConsoleColor::BrightMagenta;
    default:            return ConsoleColor::Reset;
  }
}

static std::string timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto itt = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream ss;
  ss << std::put_time(std::localtime(&itt), "%Y-%m-%d %H:%M:%S") << '.'
     << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

static std::string level_to_string(LogLevel lvl) {
  switch (lvl) {
    case LogLevel::Trace:
      return "TRACE";
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

static std::string format_as_log(LogLevel lvl, const std::string &msg) {
  std::ostringstream oss;
  oss << ConsoleColor::BrightCyan << "Izotrox> " << ConsoleColor::Reset << "["
      << timestamp() << "]"
      << "(" << level_to_string(lvl) << ") " << color_for_level(lvl) << msg
      << ConsoleColor::Reset;
  return oss.str();
}

static std::string format_as_log_no_color(LogLevel lvl, const std::string &msg) {
  std::ostringstream oss;
  oss << "Izotrox> [" << timestamp() << "]"
      << "(" << level_to_string(lvl) << ") " << msg;
  return oss.str();
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
        std::lock_guard<std::mutex> lock(m_log_mutex);
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
        info("Logging to file {}", filename_str);
    }
}

void Logger::log(LogLevel lvl, const std::string &msg) {
  std::lock_guard<std::mutex> lock(m_log_mutex);
  std::cout << format_as_log(lvl, msg) << std::endl;

  if (m_log_file && m_log_file->stream.good()) {
      m_log_file->stream << format_as_log_no_color(lvl, msg) << std::endl;
  }
}

}
