#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T, typename Allocator = std::allocator<T>> class Vector {
  using Traits = std::allocator_traits<Allocator>;

public:
  Vector(const Allocator &allocator = Allocator{})
      : ptr_(nullptr), size_(0), capacity_(0), allocator_(allocator) {}
  Vector(size_t size, const Allocator &allocator = Allocator{})
      : size_(size), capacity_(size), allocator_(allocator) {
    if (capacity_ > 0)
      ptr_ = Traits::allocate(allocator_, capacity_);
    for (size_t i = 0; i < size_; i++)
      Traits::construct(allocator_, ptr_ + i);
  }

  Vector(const Vector &other)
      : size_(other.size_), capacity_(other.capacity_),
        allocator_(other.allocator_) {
    ptr_ = capacity_ == 0 ? nullptr : Traits::allocate(allocator_, capacity_);
    for (size_t i = 0; i < size_; i++)
      Traits::construct(allocator_, ptr_ + i, other.ptr_[i]);
  }
  Vector(Vector &&other)
      : ptr_(other.ptr_), size_(other.size_), capacity_(other.capacity_),
        allocator_(std::move(other.allocator_)) {
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
      Traits::destroy(allocator_, ptr_ + (i - 1));
    Traits::deallocate(allocator_, ptr_, capacity_);
  }

  void push_back(const T &v) {
    ensure_capacity(size_ + 1);
    Traits::construct(allocator_, ptr_ + size_, v);
    size_++;
  }

  void push_back(T &&v) {
    ensure_capacity(size_ + 1);
    Traits::construct(allocator_, ptr_ + size_, std::move(v));
    size_++;
  }

  template <typename... Args> void emplace_back(Args &&...args) {
    ensure_capacity(size_ + 1);
    Traits::construct(allocator_, ptr_ + size_, std::forward<Args>(args)...);
    size_++;
  }

  void pop_back() {
    Traits::destroy(allocator_, ptr_ + size_ - 1);
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
      Traits::destroy(allocator_, ptr_ + pos);
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
      Traits::construct(allocator_, pos, value);
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
      Traits::construct(allocator_, pos, std::move(value));
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
        Traits::construct(allocator_, pos + i, value);
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
    swap(allocator_, other.allocator_);
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
        Traits::construct(allocator_, ptr_ + i);
    } else {
      for (size_t i = size_; i > new_size; i--)
        Traits::destroy(allocator_, ptr_ + i - 1);
    }
    size_ = new_size;
  }

  Allocator &get_allocator() { return allocator_; }
  const Allocator &get_allocator() const { return allocator_; }

private:
  void ensure_capacity(size_t new_capacity) {
    if (new_capacity <= capacity_)
      return;

    size_t old_capacity = capacity_;
    capacity_ = std::max(new_capacity, capacity_ * 2);
    T *new_ptr = Traits::allocate(allocator_, capacity_);
    size_t constructed = 0;
    try {
      for (; constructed < size_; constructed++)
        Traits::construct(allocator_, new_ptr + constructed,
                          std::move_if_noexcept(ptr_[constructed]));
    } catch (...) {
      for (size_t i = constructed; i > 0; i--)
        Traits::destroy(allocator_, new_ptr + i - 1);
      Traits::deallocate(allocator_, new_ptr, capacity_);
      capacity_ = old_capacity;
      throw;
    }

    for (size_t i = size_; i > 0; i--)
      Traits::destroy(allocator_, ptr_ + i - 1);
    Traits::deallocate(allocator_, ptr_, old_capacity);
    ptr_ = new_ptr;
  }

  // [first, last) guaranteed to exist
  void move_range(T *first, T *last, T *pos) {
    for (; first != last; first++, pos++) {
      if (pos < end())
        *pos = std::move(*first);
      else
        Traits::construct(allocator_, pos, std::move(*first));
    }
  }

  void move_range_backward(T *first, T *last, T *pos) {
    pos = last - first + pos;
    for (; last != first; last--, pos--) {
      if (pos - 1 < end())
        *(pos - 1) = std::move(*(last - 1));
      else
        Traits::construct(allocator_, pos - 1, std::move(*(last - 1)));
    }
  }

  T *ptr_;
  size_t size_;
  size_t capacity_;

  Allocator allocator_;
};