#pragma once
#include "logger.h"
#include "safe_logger.h"
#include <string>

class ProgramInfo {
public:
  ProgramInfo() {
    // This constructor uses g_logger, but is g_logger initialized yet?
    // g_logger.log("ProgramInfo initialized");
    g_safe_logger.log("ProgramInfo initialized safely"); 
  }

  std::string getName() const { return "MyProgram"; }

private:
  std::string name = "MyProgram";
};

// Global program info instance - When is this initialized?
extern ProgramInfo g_program_info;
