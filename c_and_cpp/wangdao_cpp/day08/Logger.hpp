#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

enum class LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };

class Logger {
public:
  static Logger &getInstance() {
    static Logger instance;
    return instance;
  }

  // Delete copy/move
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // Set minimum log level
  void setLevel(LogLevel level) { minLevel_ = level; }

  // Set output file
  bool setOutputFile(const std::string &filename) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.is_open()) {
      file_.close();
    }

    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
      std::cerr << "Failed to open log file: " << filename << std::endl;
      return false;
    }

    filename_ = filename;
    return true;
  }

  // Enable/disable console output
  void setConsoleOutput(bool enabled) { consoleOutput_ = enabled; }

  // Main log function
  void log(LogLevel level, const std::string &file, int line,
           const std::string &message) {
    if (level < minLevel_) {
      return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    std::string output = formatMessage(level, file, line, message);

    // Write to file
    if (file_.is_open()) {
      file_ << output << std::endl;
      file_.flush();
    }

    // Write to console
    if (consoleOutput_) {
      if (level >= LogLevel::ERROR) {
        std::cerr << output << std::endl;
      } else {
        std::cout << output << std::endl;
      }
    }
  }

private:
  Logger() : minLevel_(LogLevel::DEBUG), consoleOutput_(true) {}

  ~Logger() {
    if (file_.is_open()) {
      file_.close();
    }
  }

  std::string formatMessage(LogLevel level, const std::string &file, int line,
                            const std::string &message) {
    std::ostringstream oss;

    // Timestamp
    oss << "[" << getTimestamp() << "] ";

    // Level
    oss << "[" << std::setw(5) << levelToString(level) << "] ";

    // File and line
    oss << "[" << extractFilename(file) << ":" << line << "] ";

    // Message
    oss << message;

    return oss.str();
  }

  std::string getTimestamp() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    return std::string(buffer);
  }

  std::string levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARN:
      return "WARN";
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::FATAL:
      return "FATAL";
    default:
      return "?????";
    }
  }

  std::string extractFilename(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
      return path.substr(pos + 1);
    }
    return path;
  }

private:
  std::ofstream file_;
  std::string filename_;
  LogLevel minLevel_;
  bool consoleOutput_;
  std::mutex mutex_;
};

// Convenient macros
#define LOG_DEBUG(msg)                                                         \
  Logger::getInstance().log(LogLevel::DEBUG, __FILE__, __LINE__, msg)
#define LOG_INFO(msg)                                                          \
  Logger::getInstance().log(LogLevel::INFO, __FILE__, __LINE__, msg)
#define LOG_WARN(msg)                                                          \
  Logger::getInstance().log(LogLevel::WARN, __FILE__, __LINE__, msg)
#define LOG_ERROR(msg)                                                         \
  Logger::getInstance().log(LogLevel::ERROR, __FILE__, __LINE__, msg)
#define LOG_FATAL(msg)                                                         \
  Logger::getInstance().log(LogLevel::FATAL, __FILE__, __LINE__, msg)

// Format support (requires C++11)
#include <cstdarg>
#include <memory>

inline std::string formatString(const char *format, ...) {
  va_list args;
  va_start(args, format);

  va_list args_copy;
  va_copy(args_copy, args);
  int size = vsnprintf(nullptr, 0, format, args_copy) + 1;
  va_end(args_copy);

  std::unique_ptr<char[]> buffer(new char[size]);
  vsnprintf(buffer.get(), size, format, args);
  va_end(args);

  return std::string(buffer.get());
}

#define LOG_DEBUG_F(fmt, ...) LOG_DEBUG(formatString(fmt, ##__VA_ARGS__))
#define LOG_INFO_F(fmt, ...) LOG_INFO(formatString(fmt, ##__VA_ARGS__))
#define LOG_WARN_F(fmt, ...) LOG_WARN(formatString(fmt, ##__VA_ARGS__))
#define LOG_ERROR_F(fmt, ...) LOG_ERROR(formatString(fmt, ##__VA_ARGS__))
#define LOG_FATAL_F(fmt, ...) LOG_FATAL(formatString(fmt, ##__VA_ARGS__))

#endif // LOGGER_HPP