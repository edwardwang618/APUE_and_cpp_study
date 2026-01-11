// shared_data.hpp - Common header
#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include <atomic>
#include <cstring>

struct SharedData {
  std::atomic<int> counter;
  std::atomic<bool> ready;
  char message[256];

  void set_message(const char *msg) {
    std::strncpy(message, msg, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';
  }
};

constexpr const char *SHM_NAME = "/my_cpp_shm";
constexpr size_t SHM_SIZE = 4096;

#endif