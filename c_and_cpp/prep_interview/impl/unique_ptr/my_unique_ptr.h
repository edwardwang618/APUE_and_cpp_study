#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T> class UniquePtr {
public:
  constexpr UniquePtr() : ptr_(nullptr) {}
  constexpr explicit UniquePtr(T *ptr) : ptr_(ptr) {}
  UniquePtr(UniquePtr &&other) noexcept : ptr_(other.release()) {}

  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  ~UniquePtr() { delete ptr_; }

  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.release();
    }
    return *this;
  }

  UniquePtr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  T *get() const noexcept { return ptr_; }

  T *release() noexcept {
    T *p = ptr_;
    ptr_ = nullptr;
    return p;
  }

  void reset(T *ptr = nullptr) noexcept {
    if (ptr_ != ptr) {
      delete ptr_;
      ptr_ = ptr;
    }
  }

  void swap(UniquePtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
  }

  T &operator*() const noexcept { return *ptr_; }
  T *operator->() const noexcept { return ptr_; }
  explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
  T *ptr_;
};

template <typename T, typename... Args>
  requires(!std::is_array_v<T>)
UniquePtr<T> make_unique(Args &&...args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T> class UniquePtr<T[]> {
public:
  constexpr UniquePtr() noexcept : ptr_(nullptr) {}
  constexpr explicit UniquePtr(T *ptr) : ptr_(ptr) {}
  UniquePtr(UniquePtr &&other) noexcept : ptr_(other.release()) {}

  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.release();
    }
    return *this;
  }

  UniquePtr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  ~UniquePtr() { delete[] ptr_; }

  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  T *get() const noexcept { return ptr_; }

  T *release() noexcept {
    T *p = ptr_;
    ptr_ = nullptr;
    return p;
  }

  void reset(T *ptr = nullptr) noexcept {
    if (ptr_ != ptr) {
      delete[] ptr_;
      ptr_ = ptr;
    }
  }

  void swap(UniquePtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
  }

  T &operator[](size_t i) const noexcept { return ptr_[i]; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
  T *ptr_;
};

template <typename T>
  requires(std::is_unbounded_array_v<T>)
UniquePtr<T> make_unique(size_t size) {
  using Elem = std::remove_extent_t<T>;
  return UniquePtr<T>(new Elem[size]{});
}

template <typename T, typename... Args>
  requires(std::is_bounded_array_v<T>)
UniquePtr<T> make_unique(Args &&...) = delete;