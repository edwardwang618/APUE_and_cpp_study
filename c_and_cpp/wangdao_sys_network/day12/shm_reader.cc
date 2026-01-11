// shm_reader.cpp
#include "shared_data.hpp"
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  // Open existing shared memory
  int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
  if (fd == -1) {
    std::perror("shm_open (run writer first!)");
    return 1;
  }

  void *ptr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    std::perror("mmap");
    return 1;
  }

  const SharedData *data = static_cast<const SharedData *>(ptr);

  std::cout << "Reader: Starting...\n";

  int last_counter = 0;
  for (int i = 0; i < 20; ++i) {
    if (data->ready.load() && data->counter.load() != last_counter) {
      last_counter = data->counter.load();
      std::cout << "Reader: counter=" << last_counter << ", msg='"
                << data->message << "'\n";
    }
    usleep(300000);
  }

  munmap(ptr, SHM_SIZE);
  close(fd);

  std::cout << "Reader: Done!\n";
  return 0;
}