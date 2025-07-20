#pragma once
#include <string>
#include <iostream>

class Logger {
public:
  Logger() {
    // This constructor might use uninitialized program_info!
    initialized = true;
  }

  void log(const std::string &msg) {
    if (initialized) {
      // do logging
      std::cout << "Log from Logger: " << msg << std::endl;
    }
  }

private:
  bool initialized = false;
};

// Global logger instance - When is this initialized?
extern Logger g_logger;
