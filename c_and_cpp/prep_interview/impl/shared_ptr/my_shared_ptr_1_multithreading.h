#include <atomic>
#include <cstddef>
#include <utility>

template <typename T> class SharedPtr {
public:
  constexpr SharedPtr() noexcept : ptr_(nullptr), count_(nullptr) {}

  explicit SharedPtr(T *ptr)
      : ptr_(ptr), count_(ptr ? new std::atomic_size_t{1} : nullptr) {}

  constexpr SharedPtr(std::nullptr_t) noexcept
      : ptr_(nullptr), count_(nullptr) {}

  ~SharedPtr() {
    if (count_ && count_->fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete ptr_;
      delete count_;
    }
  }

  SharedPtr(const SharedPtr &other) noexcept
      : ptr_(other.ptr_), count_(other.count_) {
    if (count_)
      count_->fetch_add(1, std::memory_order_relaxed);
  }

  SharedPtr &operator=(const SharedPtr &other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      count_ = other.count_;
      if (count_)
        count_->fetch_add(1, std::memory_order_relaxed);
    }
    return *this;
  }

  SharedPtr(SharedPtr &&other) noexcept
      : ptr_(other.ptr_), count_(other.count_) {
    other.ptr_ = nullptr;
    other.count_ = nullptr;
  }

  SharedPtr &operator=(SharedPtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      count_ = other.count_;
      other.ptr_ = nullptr;
      other.count_ = nullptr;
    }
    return *this;
  }

  void reset(T *p = nullptr) noexcept {
    if (count_ && count_->fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete ptr_;
      delete count_;
    }
    ptr_ = p;
    count_ = p ? new std::atomic_size_t{1} : nullptr;
  }

  T *get() const noexcept { return ptr_; }
  T &operator*() const noexcept { return *ptr_; }
  T *operator->() const noexcept { return ptr_; }

  std::size_t use_count() const noexcept {
    return count_ ? count_->load(std::memory_order_relaxed) : 0;
  }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

  void swap(SharedPtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(count_, other.count_);
  }

private:
  T *ptr_;
  std::atomic_size_t *count_;
};

template <typename T> void swap(SharedPtr<T> &a, SharedPtr<T> &b) noexcept {
  a.swap(b);
}

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args &&...args) {
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}