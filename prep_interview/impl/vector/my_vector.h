#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T> class Vector {
public:
  Vector() : ptr_(nullptr), size_(0), capacity_(0) {}
  Vector(size_t size)
      : ptr_(static_cast<T *>(malloc(size * sizeof(T)))), size_(size),
        capacity_(size) {
    if (!ptr_)
      throw std::bad_alloc();
    for (size_t i = 0; i < size_; i++)
      new (ptr_ + i) T{};
  }

  Vector(const Vector &other) : size_(other.size_), capacity_(other.capacity_) {
    ptr_ = capacity_ == 0 ? nullptr
                          : static_cast<T *>(malloc(capacity_ * sizeof(T)));
    for (size_t i = 0; i < size_; i++)
      new (ptr_ + i) T(other.ptr_[i]);
  }
  Vector(Vector &&other)
      : ptr_(other.ptr_), size_(other.size_), capacity_(other.capacity_) {
    other.ptr_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  Vector &operator=(Vector other) {
    swap(other);
    return *this;
  }

  ~Vector() {
    for (size_t i = size_; i; i--)
      ptr_[i - 1].~T();
    free(ptr_);
  }

  void push_back(const T &v) {
    ensure_capacity(size_ + 1);
    new (ptr_ + size_) T(v);
    size_++;
  }

  void push_back(T &&v) {
    ensure_capacity(size_ + 1);
    new (ptr_ + size_) T(std::move(v));
    size_++;
  }

  template <typename... Args> void emplace_back(Args &&...args) {
    ensure_capacity(size_ + 1);
    new (ptr_ + size_) T(std::forward<Args>(args)...);
    size_++;
  }

  void pop_back() {
    ptr_[size_ - 1].~T();
    size_--;
  }

  void clear() { resize(0); }

  T *erase(T *pos) { return erase(pos, pos + 1); }

  T *erase(T *first, T *last) {
    if (first >= last)
      return first;
    move_range(last, end(), first);
    size_t new_size = size_ - (last - first);
    for (size_t pos = new_size; pos < size_; pos++)
      (ptr_ + pos)->~T();
    size_ = new_size;
    return first;
  }

  T *insert(T *pos, const T &value) {
    T *old_ptr = ptr_;
    ensure_capacity(size_ + 1);
    pos = pos - old_ptr + ptr_;
    move_range_backward(pos, end(), pos + 1);
    if (pos < end())
      *pos = value;
    else
      new (pos) T(value);
    size_++;
    return pos;
  }

  T *insert(T *pos, T &&value) {
    T *old_ptr = ptr_;
    ensure_capacity(size_ + 1);
    pos = pos - old_ptr + ptr_;
    move_range_backward(pos, end(), pos + 1);
    if (pos < end())
      *pos = std::move(value);
    else
      new (pos) T(std::move(value));
    size_++;
    return pos;
  }

  T *insert(T *pos, size_t count, const T &value) {
    T *old_ptr = ptr_;
    ensure_capacity(size_ + count);
    pos = pos - old_ptr + ptr_;
    move_range_backward(pos, end(), pos + count);
    for (size_t i = 0; i < count; i++)
      if (pos + i < end())
        *(pos + i) = value;
      else
        new (pos + i) T(value);
    size_ += count;
    return pos;
  }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  bool empty() const { return size_ == 0; }

  T &operator[](size_t i) { return ptr_[i]; }
  const T &operator[](size_t i) const { return ptr_[i]; }

  T &at(size_t i) {
    if (i >= size_)
      throw std::out_of_range("index out of range");
    return ptr_[i];
  }
  const T &at(size_t i) const {
    if (i >= size_)
      throw std::out_of_range("index out of range");
    return ptr_[i];
  }

  void swap(Vector &other) noexcept {
    using std::swap;
    swap(ptr_, other.ptr_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
  }

  T *begin() { return ptr_; }
  const T *begin() const { return ptr_; }
  T *end() { return ptr_ + size_; }
  const T *end() const { return ptr_ + size_; }

  void reserve(size_t new_capacity) { ensure_capacity(new_capacity); }

  void resize(size_t new_size) {
    ensure_capacity(new_size);
    if (new_size > size_) {
      for (size_t i = size_; i < new_size; i++)
        new (ptr_ + i) T{};
    } else {
      for (size_t i = size_; i > new_size; i--)
        ptr_[i - 1].~T();
    }
    size_ = new_size;
  }

private:
  void ensure_capacity(size_t new_capacity) {
    if (new_capacity <= capacity_)
      return;

    capacity_ = std::max(new_capacity, capacity_ * 2);
    T *new_ptr_ = static_cast<T *>(malloc(sizeof(T) * capacity_));
    if (!new_ptr_)
      throw std::bad_alloc();
    for (size_t i = 0; i < size_; i++) {
      new (new_ptr_ + i) T(std::move(ptr_[i]));
      ptr_[i].~T();
    }
    free(ptr_);
    ptr_ = new_ptr_;
  }

  // [first, last) guaranteed to exist
  void move_range(T *first, T *last, T *pos) {
    for (; first != last; first++, pos++) {
      if (pos < end())
        *pos = std::move(*first);
      else
        new (pos) T(std::move(*first));
    }
  }

  void move_range_backward(T *first, T *last, T *pos) {
    pos = last - first + pos;
    for (; last != first; last--, pos--) {
      if (pos - 1 < end())
        *(pos - 1) = std::move(*(last - 1));
      else
        new (pos - 1) T(std::move(*(last - 1)));
    }
  }

  T *ptr_;
  size_t size_;
  size_t capacity_;
};