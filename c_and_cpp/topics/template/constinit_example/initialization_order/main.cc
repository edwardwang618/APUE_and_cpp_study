#include "logger.h"
#include "program_info.h"
#include "safe_logger.h"
#include <iostream>
#include <thread>

int main() {
  // Try to use the potentially uninitialized logger
  g_logger.log("Starting program");

  // Try to access program info (which uses logger in constructor)
  std::cout << "Program name: " << g_program_info.getName() << std::endl;

  // Using the safe logger with constinit
  g_safe_logger.log("Using safe logger - guaranteed to be initialized");

  return 0;
}
