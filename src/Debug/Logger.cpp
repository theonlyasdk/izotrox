#include <Debug/Logger.hpp>
#include <Utils/ASCIIColor.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace Izo {

static const char *color_for_level(Level lvl) {
  using namespace ASCIIColor;
  switch (lvl) {
  case Level::Trace:
    return BrightBlack;
  case Level::Debug:
    return BrightBlue;
  case Level::Info:
    return BrightGreen;
  case Level::Warn:
    return BrightYellow;
  case Level::Error:
    return BrightRed;
  case Level::Fatal:
    return BrightMagenta;
  default:
    return Reset;
  }
}

struct Logger::LogFile {
    std::ofstream stream;
};

Logger::Logger() : min_level_(Level::Info) {}
Logger::~Logger() = default;

Logger &Logger::the() {
  static Logger s_instance;
  return s_instance;
}

void Logger::set_level(Level lvl) { min_level_ = lvl; }

void Logger::enable_logging_to_file() {
    std::string filename_str;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (m_log_file) return;

        using namespace std::chrono;
        auto now = system_clock::now();
        auto itt = system_clock::to_time_t(now);
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
        info("Enabled logging to " + filename_str);
    }
}

void Logger::log(Level lvl, const std::string &msg) {
  if (lvl < min_level_)
    return;
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
void Logger::fatal(const std::string &msg) { log(Level::Fatal, msg); }

std::string Logger::format(Level lvl, const std::string &msg) {
  std::ostringstream oss;
  oss << ASCIIColor::BrightCyan << "Izotrox> " << ASCIIColor::Reset << "["
      << timestamp() << "]"
      << "(" << level_to_string(lvl) << ") " << color_for_level(lvl) << msg
      << ASCIIColor::Reset;
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
