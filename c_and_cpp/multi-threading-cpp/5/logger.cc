#include <iostream>

struct Logger {
  Logger() { std::cout << "Logger created\n"; }
  ~Logger() { std::cout << "Logger destroyed\n"; }
  void log(const char *msg) { std::cout << msg << "\n"; }

  static Logger &getInstance() {
    static Logger instance;
    return instance;
  }
};

struct Database {
  Database() { Logger::getInstance().log("DB created"); }
  ~Database() { Logger::getInstance().log("DB destroyed"); } // DANGER!

  static Database &getInstance() {
    static Database instance;
    return instance;
  }
};

int main() { Database::getInstance(); }