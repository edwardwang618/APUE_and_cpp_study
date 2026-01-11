#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T> struct DefaultDeleter {
  constexpr DefaultDeleter() noexcept = default;

  template <typename U>
    requires std::is_convertible_v<U *, T *>
  DefaultDeleter(const DefaultDeleter<U> &) noexcept {}

  void operator()(T *ptr) const noexcept {
    static_assert(sizeof(T), "cannot delete incomplete type");
    delete ptr;
  }
};

template <typename T> struct DefaultDeleter<T[]> {
  constexpr DefaultDeleter() noexcept = default;

  void operator()(T *ptr) const noexcept {
    static_assert(sizeof(T), "cannot delete incomplete type");
    delete[] ptr;
  }
};

template <typename T, typename Deleter = DefaultDeleter<T>> class UniquePtr {
public:
  constexpr UniquePtr() noexcept : ptr_(nullptr), deleter_() {}
  constexpr UniquePtr(std::nullptr_t) noexcept : ptr_(nullptr), deleter_() {}

  explicit UniquePtr(T *ptr) : ptr_(ptr), deleter_() {}

  UniquePtr(T *ptr, const Deleter &deleter) : ptr_(ptr), deleter_(deleter) {}
  UniquePtr(T *ptr, Deleter &&deleter) noexcept
      : ptr_(ptr), deleter_(std::move(deleter)) {}

  UniquePtr(UniquePtr &&other) noexcept
      : ptr_(other.release()), deleter_(std::move(other.deleter_)) {}

  template <typename U, typename D>
    requires std::is_convertible_v<U *, T *> &&
                 std::is_convertible_v<D, Deleter>
  UniquePtr(UniquePtr<U, D> &&other) noexcept
      : ptr_(other.release()), deleter_(std::move(other.deleter_)) {}

  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  ~UniquePtr() {
    if (ptr_)
      deleter_(ptr_);
  }

  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.release();
      deleter_ = std::move(other.deleter_);
    }
    return *this;
  }

  template <typename U, typename D>
    requires std::is_convertible_v<U *, T *> &&
             std::is_convertible_v<D, Deleter>
  UniquePtr &operator=(UniquePtr<U, D> &&other) {
    reset();
    ptr_ = other.release();
    deleter_ = std::move(other.deleter_);
    return *this;
  }

  UniquePtr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  T *get() const noexcept { return ptr_; }

  Deleter &get_deleter() noexcept { return deleter_; }
  const Deleter &get_deleter() const noexcept { return deleter_; }

  T *release() noexcept {
    T *p = ptr_;
    ptr_ = nullptr;
    return p;
  }

  void reset(T *ptr = nullptr) noexcept {
    if (ptr_ != ptr) {
      if (ptr_)
        deleter_(ptr_);
      ptr_ = ptr;
    }
  }

  void swap(UniquePtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(deleter_, other.deleter_);
  }

  T &operator*() const noexcept { return *ptr_; }
  T *operator->() const noexcept { return ptr_; }
  explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
  T *ptr_;
  [[no_unique_address]] Deleter deleter_;

  template <typename U, typename D> friend class UniquePtr;
};

template <typename T, typename... Args>
  requires(!std::is_array_v<T>)
UniquePtr<T> make_unique(Args &&...args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename Deleter> class UniquePtr<T[], Deleter> {
public:
  constexpr UniquePtr() noexcept : ptr_(nullptr), deleter_() {}
  constexpr UniquePtr(std::nullptr_t) noexcept : ptr_(nullptr), deleter_() {}
  explicit UniquePtr(T *ptr) : ptr_(ptr), deleter_() {}

  UniquePtr(T *ptr, const Deleter &deleter) : ptr_(ptr), deleter_(deleter) {}
  UniquePtr(T *ptr, Deleter &&deleter) noexcept
      : ptr_(ptr), deleter_(std::move(deleter)) {}

  UniquePtr(UniquePtr &&other) noexcept
      : ptr_(other.release()), deleter_(std::move(other.deleter_)) {}

  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.release();
      deleter_ = std::move(other.deleter_);
    }
    return *this;
  }

  UniquePtr &operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  ~UniquePtr() {
    if (ptr_)
      deleter_(ptr_);
  }

  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  T *get() const noexcept { return ptr_; }
  Deleter &get_deleter() noexcept { return deleter_; }
  const Deleter &get_deleter() const noexcept { return deleter_; }

  T *release() noexcept {
    T *p = ptr_;
    ptr_ = nullptr;
    return p;
  }

  void reset(T *ptr = nullptr) noexcept {
    if (ptr_ != ptr) {
      if (ptr_)
        deleter_(ptr_);
      ptr_ = ptr;
    }
  }

  void swap(UniquePtr &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(deleter_, other.deleter_);
  }

  T &operator[](size_t i) const noexcept { return ptr_[i]; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
  T *ptr_;
  [[no_unique_address]] Deleter deleter_;

  template <typename U, typename D> friend class UniquePtr;
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