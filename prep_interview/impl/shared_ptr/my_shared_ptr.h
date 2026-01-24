#include <cstddef>
#include <iostream>
#include <utility>

struct ControlBlock {
  std::size_t strong_count;
  std::size_t weak_count;
};

template <typename T> class WeakPtr;

template <typename T> class SharedPtr {
public:
  constexpr SharedPtr() noexcept : ptr_(nullptr), ctrl_(nullptr) {}
  explicit SharedPtr(T *ptr) noexcept
      : ptr_(ptr), ctrl_(new ControlBlock{1, 1}) {}
  SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), ctrl_(nullptr) {}

  ~SharedPtr() {
    if (ctrl_ && --ctrl_->strong_count == 0) {
      delete ptr_;
      if (--ctrl_->weak_count == 0)
        delete ctrl_;
    }
  }

  SharedPtr(const SharedPtr &other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_)
      ctrl_->strong_count++;
  }

  SharedPtr &operator=(const SharedPtr &other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_)
        ctrl_->strong_count++;
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
    if (ctrl_ && --ctrl_->strong_count == 0) {
      delete ptr_;
      if (--ctrl_->weak_count == 0)
        delete ctrl_;
    }
    ptr_ = p;
    ctrl_ = nullptr;
    if (ptr_)
      ctrl_ = new ControlBlock{1, 1};
  }

  T *get() const { return ptr_; }
  T &operator*() const { return *ptr_; }
  T *operator->() const { return ptr_; }
  std::size_t use_count() const { return ctrl_ ? ctrl_->strong_count : 0; }
  explicit operator bool() const { return ptr_ != nullptr; }

  void swap(SharedPtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(ctrl_, other.ctrl_);
  }

private:
  SharedPtr(T *ptr, ControlBlock *ctrl) noexcept : ptr_(ptr), ctrl_(ctrl) {
    if (ctrl_)
      ctrl_->strong_count++;
  }

  T *ptr_;
  ControlBlock *ctrl_;

  friend class WeakPtr<T>;
};

template <typename T> void swap(SharedPtr<T> &a, SharedPtr<T> &b) noexcept {
  a.swap(b);
}

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args &&...args) {
  return SharedPtr(new T(std::forward<Args>(args)...));
}

template <typename T> class WeakPtr {
public:
  WeakPtr() : ptr_(nullptr), ctrl_(nullptr) {}
  WeakPtr(const SharedPtr<T> &sp) : ptr_(sp.ptr_), ctrl_(sp.ctrl_) {
    if (ctrl_)
      ctrl_->weak_count++;
  }

  ~WeakPtr() { reset(); }

  WeakPtr(const WeakPtr &other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_)
      ctrl_->weak_count++;
  }

  WeakPtr(WeakPtr &&other) noexcept : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    other.ptr_ = nullptr;
    other.ctrl_ = nullptr;
  }

  WeakPtr &operator=(const WeakPtr &other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_)
        ctrl_->weak_count++;
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
      ctrl_->weak_count++;
    return *this;
  }

  void reset() noexcept {
    if (ctrl_ && --ctrl_->weak_count == 0)
      delete ctrl_;
    ptr_ = nullptr;
    ctrl_ = nullptr;
  }

  SharedPtr<T> lock() const {
    if (!ctrl_ || ctrl_->strong_count == 0)
      return nullptr;
    return SharedPtr(ptr_, ctrl_);
  }
  std::size_t use_count() const { return ctrl_ ? ctrl_->strong_count : 0; }
  bool expired() const { return !ctrl_ || ctrl_->strong_count == 0; }

private:
  T *ptr_;
  ControlBlock *ctrl_;

  friend class SharedPtr<T>;
};