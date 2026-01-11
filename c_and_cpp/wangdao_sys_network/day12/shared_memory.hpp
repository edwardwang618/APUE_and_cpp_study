// shared_memory.hpp - Reusable RAII wrapper
#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

template <typename T> class SharedMemory {
public:
  // Create new shared memory
  static SharedMemory create(const std::string &name) {
    return SharedMemory(name, true);
  }

  // Open existing shared memory
  static SharedMemory open(const std::string &name) {
    return SharedMemory(name, false);
  }

  ~SharedMemory() {
    if (ptr_ != nullptr) {
      munmap(ptr_, sizeof(T));
    }
    if (fd_ != -1) {
      close(fd_);
    }
    if (owner_) {
      shm_unlink(name_.c_str());
    }
  }

  // Non-copyable
  SharedMemory(const SharedMemory &) = delete;
  SharedMemory &operator=(const SharedMemory &) = delete;

  // Movable
  SharedMemory(SharedMemory &&other) noexcept
      : name_(std::move(other.name_)), fd_(other.fd_), ptr_(other.ptr_),
        owner_(other.owner_) {
    other.fd_ = -1;
    other.ptr_ = nullptr;
    other.owner_ = false;
  }

  SharedMemory &operator=(SharedMemory &&other) noexcept {
    if (this != &other) {
      if (ptr_)
        munmap(ptr_, sizeof(T));
      if (fd_ != -1)
        close(fd_);
      if (owner_)
        shm_unlink(name_.c_str());

      name_ = std::move(other.name_);
      fd_ = other.fd_;
      ptr_ = other.ptr_;
      owner_ = other.owner_;

      other.fd_ = -1;
      other.ptr_ = nullptr;
      other.owner_ = false;
    }
    return *this;
  }

  T *get() { return ptr_; }
  const T *get() const { return ptr_; }
  T *operator->() { return ptr_; }
  const T *operator->() const { return ptr_; }
  T &operator*() { return *ptr_; }
  const T &operator*() const { return *ptr_; }

private:
  SharedMemory(const std::string &name, bool create)
      : name_(name), owner_(create) {

    int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
    fd_ = shm_open(name_.c_str(), flags, 0666);
    if (fd_ == -1) {
      throw std::runtime_error("shm_open failed: " +
                               std::string(strerror(errno)));
    }

    if (create) {
      if (ftruncate(fd_, sizeof(T)) == -1) {
        close(fd_);
        shm_unlink(name_.c_str());
        throw std::runtime_error("ftruncate failed");
      }
    }

    void *addr =
        mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (addr == MAP_FAILED) {
      close(fd_);
      if (create)
        shm_unlink(name_.c_str());
      throw std::runtime_error("mmap failed");
    }

    if (create) {
      ptr_ = new (addr) T{}; // Placement new
    } else {
      ptr_ = static_cast<T *>(addr);
    }
  }

  std::string name_;
  int fd_ = -1;
  T *ptr_ = nullptr;
  bool owner_ = false;
};

#endif