#include <atomic>
#include <cstddef>
#include <utility>

struct ControlBlock {
  std::atomic_size_t strong_count;
  std::atomic_size_t weak_count;
};

template <typename T> class WeakPtr;

template <typename T> class SharedPtr {
public:
  constexpr SharedPtr() noexcept : ptr_(nullptr), ctrl_(nullptr) {}

  explicit SharedPtr(T *ptr)
      : ptr_(ptr), ctrl_(ptr ? new ControlBlock{1, 1} : nullptr) {}

  constexpr SharedPtr(std::nullptr_t) noexcept
      : ptr_(nullptr), ctrl_(nullptr) {}

  ~SharedPtr() {
    if (ctrl_ &&
        ctrl_->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete ptr_;
      if (ctrl_->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        delete ctrl_;
    }
  }

  SharedPtr(const SharedPtr &other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_)
      ctrl_->strong_count.fetch_add(1, std::memory_order_relaxed);
  }

  SharedPtr &operator=(const SharedPtr &other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_)
        ctrl_->strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    return *this;
  }

  SharedPtr(SharedPtr &&other) noexcept : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    other.ptr_ = nullptr;
    other.ctrl_ = nullptr;
  }

  SharedPtr &operator=(SharedPtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      other.ptr_ = nullptr;
      other.ctrl_ = nullptr;
    }
    return *this;
  }

  void reset(T *p = nullptr) noexcept {
    if (ctrl_ &&
        ctrl_->strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete ptr_;
      if (ctrl_->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        delete ctrl_;
    }
    ptr_ = p;
    ctrl_ = p ? new ControlBlock{1, 1} : nullptr;
  }

  T *get() const noexcept { return ptr_; }
  T &operator*() const noexcept { return *ptr_; }
  T *operator->() const noexcept { return ptr_; }

  std::size_t use_count() const noexcept {
    return ctrl_ ? ctrl_->strong_count.load(std::memory_order_relaxed) : 0;
  }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

  void swap(SharedPtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(ctrl_, other.ctrl_);
  }

private:
  T *ptr_;
  ControlBlock *ctrl_;

  template <typename U> friend class WeakPtr;
};

template <typename T> void swap(SharedPtr<T> &a, SharedPtr<T> &b) noexcept {
  a.swap(b);
}

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args &&...args) {
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T> class WeakPtr {
public:
  constexpr WeakPtr() noexcept : ptr_(nullptr), ctrl_(nullptr) {}
  WeakPtr(const SharedPtr<T> &sp) noexcept : ptr_(sp.ptr_), ctrl_(sp.ctrl_) {
    if (ctrl_)
      ctrl_->weak_count.fetch_add(1, std::memory_order_relaxed);
  }
  WeakPtr(const WeakPtr &other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_)
      ctrl_->weak_count.fetch_add(1, std::memory_order_relaxed);
  }

  WeakPtr(WeakPtr &&other) noexcept : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    other.ptr_ = nullptr;
    other.ctrl_ = nullptr;
  }

  ~WeakPtr() { reset(); }

  WeakPtr &operator=(const WeakPtr &other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_)
        ctrl_->weak_count.fetch_add(1, std::memory_order_relaxed);
    }
    return *this;
  }
  WeakPtr &operator=(WeakPtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      other.ptr_ = nullptr;
      other.ctrl_ = nullptr;
    }
    return *this;
  }
  WeakPtr &operator=(const SharedPtr<T> &sp) noexcept {
    reset();
    ptr_ = sp.ptr_;
    ctrl_ = sp.ctrl_;
    if (ctrl_)
      ctrl_->weak_count.fetch_add(1, std::memory_order_relaxed);
    return *this;
  }

  std::size_t use_count() const noexcept {
    return ctrl_ ? ctrl_->strong_count.load(std::memory_order_acq_rel) : 0;
  }
  bool expired() const noexcept { return use_count() == 0; }

  SharedPtr<T> lock() const noexcept {
    if (!ctrl_)
      return SharedPtr<T>();

    size_t count = ctrl_->strong_count.load(std::memory_order_relaxed);
    while (count > 0) {
      if (ctrl_->strong_count.compare_exchange_weak(
              count, count + 1, std::memory_order_acq_rel)) {
        SharedPtr<T> sp;
        sp.ptr_ = ptr_;
        sp.ctrl_ = ctrl_;
        return sp;
      }
    }
    return SharedPtr<T>();
  }

  void reset() noexcept {
    if (ctrl_ && ctrl_->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
      delete ctrl_;
    ptr_ = nullptr;
    ctrl_ = nullptr;
  }

  void swap(WeakPtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(ctrl_, other.ctrl_);
  }

private:
  T *ptr_;
  ControlBlock *ctrl_;
};

template <typename T> void swap(WeakPtr<T> &a, WeakPtr<T> &b) { a.swap(b); }