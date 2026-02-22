#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <vector>
#include <algorithm>

#include <Debug/Logger.hpp>
#include <Debug/ConsoleColor.hpp>

namespace Izo {

struct Logger::LogFile {
    std::ofstream stream;
};

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
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Fatal: return "FATAL";
    default:              return "UNKNOWN";
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

Logger::Logger(){}
Logger::~Logger() = default;

Logger &Logger::the() {
  static Logger s_instance;
  return s_instance;
}

void Logger::enable_logging_to_file() {
    constexpr size_t kMaxLogs = 10;
    std::string filename;
  
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        if (m_log_file) return;

        namespace fs = std::filesystem;
        fs::path log_dir = "logs";
        if (!fs::exists(log_dir)) {
            fs::create_directory(log_dir);
        }

        std::vector<fs::path> log_files;
        for (const auto& entry : fs::directory_iterator(log_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                log_files.push_back(entry.path());
            }
        }

        if (log_files.size() >= kMaxLogs) {
            std::sort(log_files.begin(), log_files.end());
            for (size_t i = 0; i <= log_files.size() - kMaxLogs; ++i) {
                fs::remove(log_files[i]);
            }
        }

        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "izotrox-" << std::put_time(std::localtime(&itt), "%Y%m%d-%H%M%S") << ".log";

        fs::path log_path = log_dir / ss.str();
        filename = log_path.string();

        m_log_file = std::make_unique<LogFile>();
        m_log_file->stream.open(log_path, std::ios::out | std::ios::app);
    }

    if (!filename.empty()) {
        LogInfo("Logging to file {}", filename);
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
