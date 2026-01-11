// shm_writer.cpp
#include "shared_data.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  // Create shared memory
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (fd == -1) {
    std::perror("shm_open");
    return 1;
  }

  if (ftruncate(fd, SHM_SIZE) == -1) {
    std::perror("ftruncate");
    return 1;
  }

  // Map to address space
  void *ptr =
      mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    std::perror("mmap");
    return 1;
  }

  // Placement new to construct object in shared memory
  SharedData *data = new (ptr) SharedData{};
  data->counter.store(0);
  data->ready.store(false);

  std::cout << "Writer: Starting...\n";

  for (int i = 1; i <= 10; ++i) {
    data->counter.store(i);
    std::string msg = "Message #" + std::to_string(i);
    data->set_message(msg.c_str());
    data->ready.store(true);

    std::cout << "Writer: counter=" << i << ", msg='" << data->message << "'\n";
    sleep(1);
  }

  // Cleanup
  munmap(ptr, SHM_SIZE);
  close(fd);
  shm_unlink(SHM_NAME);

  std::cout << "Writer: Done!\n";
  return 0;
}