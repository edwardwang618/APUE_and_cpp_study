#pragma once
#include <string_view>

// Safe version using constinit
class SafeLogger {
public:
  // Initialize with constexpr constructor
  constexpr SafeLogger() : initialized(true) {}

  void log(std::string_view msg) const {
    if (initialized) {
      // do logging
    }
  }

private:
  bool initialized;
};

// Global logger instance with guaranteed initialization order - declaration
// only
extern constinit SafeLogger g_safe_logger;
