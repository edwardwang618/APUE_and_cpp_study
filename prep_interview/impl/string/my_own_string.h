// Your implementation goes here
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

class String {
private:
  size_t size_;
  size_t capacity_;
  char *s_;

  void ensure_capacity(size_t capacity) {
    if (capacity <= capacity_)
      return;
    capacity_ = std::max(2 * capacity_, capacity);
    char *p = new char[capacity_];
    strcpy(p, s_);
    delete[] s_;
    s_ = p;
    s_[size_] = '\0';
  }

public:
  inline static constexpr size_t npos = static_cast<size_t>(-1);
  ~String() noexcept { delete[] s_; }
  String() : size_(0), capacity_(1), s_(new char[]{'\0'}) {}
  String(const char *s) : size_(strlen(s)), capacity_(size_ + 1) {
    s_ = new char[size_ + 1];
    strcpy(s_, s);
  }

  String(const char *s, size_t size) : size_(size), capacity_(size + 1) {
    s_ = new char[size_ + 1];
    memcpy(s_, s, size_ * sizeof(char));
    s_[size_] = '\0';
  }

  String(size_t n, char c) : size_(n), capacity_(size_ + 1) {
    s_ = new char[size_ + 1];
    std::fill(s_, s_ + size_, c);
    s_[size_] = '\0';
  }

  String(char c) : size_(1), capacity_(2), s_(new char[2]{c, '\0'}) {}

  String(const String &other) : size_(other.size_), capacity_(size_ + 1) {
    s_ = new char[size_ + 1];
    strcpy(s_, other.s_);
  }

  String(String &&other) noexcept
      : size_(other.size_), capacity_(other.capacity_), s_(other.s_) {
    other.s_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  String &operator=(const String &other) {
    if (this != &other) {
      size_ = other.size_;
      capacity_ = other.capacity_;
      delete[] s_;
      s_ = new char[size_ + 1];
      strcpy(s_, other.s_);
    }
    return *this;
  }

  String &operator=(String &&other) noexcept {
    if (this != &other) {
      size_ = other.size_;
      capacity_ = other.capacity_;
      delete[] s_;
      s_ = other.s_;
      other.s_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  String &operator=(const char *s) {
    size_ = strlen(s);
    ensure_capacity(size_ + 1);
    strcpy(s_, s);
    return *this;
  }

  String &operator=(char c) {
    ensure_capacity(2);
    size_ = 1;
    s_[0] = c;
    s_[1] = '\0';
    return *this;
  }

  char &operator[](size_t idx) { return s_[idx]; }

  const char &operator[](size_t idx) const { return s_[idx]; }

  char &at(size_t idx) {
    assert(idx < size_);
    if (idx < size_)
      return s_[idx];
    throw std::out_of_range("Given index out of bound");
  }

  const char &at(size_t idx) const {
    if (0 <= idx && idx < size_)
      return s_[idx];
    throw std::out_of_range("Given index out of bound");
  }

  char &front() { return s_[0]; }
  const char &front() const { return s_[0]; }

  char &back() { return s_[size_ - 1]; }
  const char &back() const { return s_[size_ - 1]; }

  const char *c_str() const { return s_; }

  const char *data() const { return s_; }

  size_t size() const { return size_; }
  size_t length() const { return size_; }

  void reserve(size_t capacity) {
    if (capacity > capacity_) {
      capacity_ = capacity;
      char *p = new char[capacity_];
      strcpy(p, s_);
      delete[] s_;
      s_ = p;
    }
  }

  size_t capacity() const { return capacity_; }

  void shrink_to_fit() {
    capacity_ = size_ + 1;
    char *p = new char[capacity_];
    strcpy(p, s_);
    delete[] s_;
    s_ = p;
  }

  void clear() {
    size_ = 0;
    s_[0] = '\0';
  }

  bool empty() const { return size_ == 0; }

  void push_back(char c) {
    ensure_capacity(size_ + 2);
    s_[size_] = c;
    s_[size_ + 1] = '\0';
    size_++;
  }

  void pop_back() {
    if (size_ > 0) {
      size_--;
      s_[size_] = '\0';
      return;
    }
    throw std::out_of_range("pop_back when empty");
  }

  String &append(const String &other) {
    ensure_capacity(size_ + other.size_ + 1);
    strcpy(s_ + size_, other.s_);
    size_ += other.size_;
    return *this;
  }

  String &operator+=(const char *other) {
    size_t len = strlen(other);
    ensure_capacity(size_ + len + 1);
    strcpy(s_ + size_, other);
    size_ += len;
    return *this;
  }

  String &operator+=(char c) {
    push_back(c);
    return *this;
  }

  String &operator+=(const String &other) {
    ensure_capacity(size_ + other.size_ + 1);
    strcpy(s_ + size_, other.s_);
    size_ += other.size_;
    return *this;
  }

  void resize(size_t size) {
    ensure_capacity(size + 1);
    size_ = size;
    s_[size_] = '\0';
  }

  void resize(size_t size, char c) {
    ensure_capacity(size + 1);
    if (size > size_)
      std::fill(s_ + size_, s_ + size, c);
    size_ = size;
    s_[size_] = '\0';
  }

  friend String operator+(const String &s1, const String &s2) {
    String s = s1;
    s += s2;
    return s;
  }

  friend String operator+(const String &s1, const char *s2) {
    String s = s1;
    s += s2;
    return s;
  }

  friend String operator+(const char *s1, const String &s2) {
    String s(s1);
    s += s2;
    return s;
  }

  bool operator==(const String &other) const { return !strcmp(s_, other.s_); }
  bool operator==(const char *other) const { return !strcmp(s_, other); }

  bool operator!=(const String &other) const { return !(*this == other); }

  bool operator<(const String &other) const { return strcmp(s_, other.s_) < 0; }
  bool operator<(const char *other) const { return strcmp(s_, other) < 0; }

  bool operator>(const String &other) const { return strcmp(s_, other.s_) > 0; }

  bool operator<=(const String &other) const {
    return strcmp(s_, other.s_) <= 0;
  }

  bool operator>=(const String &other) const {
    return strcmp(s_, other.s_) >= 0;
  }

  friend bool operator==(const char *s, const String &other) {
    return !strcmp(s, other.s_);
  }

  String substr(size_t start) { return String(s_ + start); }

  String substr(size_t start, size_t len) { return String(s_ + start, len); }

  size_t find(char c) const {
    for (size_t i = 0; i < size_; i++)
      if (s_[i] == c)
        return i;
    return String::npos;
  }

  size_t find(char c, size_t start) const {
    for (size_t i = start; i < size_; i++)
      if (s_[i] == c)
        return i;
    return String::npos;
  }

  size_t find(const String &other) const {
    for (size_t i = 0; i + other.size_ <= size_; i++) {
      size_t j;
      for (j = 0; j < other.size_; j++)
        if (s_[i + j] != other[j])
          break;
      if (j == other.size_)
        return i;
    }
    return String::npos;
  }

  class Iterator {
  private:
    char *it;

  public:
    Iterator(char *pos) : it(pos) {}
    Iterator &operator++() {
      ++it;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++it;
      return tmp;
    }
    operator const char *() { return it; }
    char &operator*() { return *it; }
    const char &operator*() const { return *it; }
    Iterator operator-(size_t offset) { return Iterator(it - offset); }
  };

  Iterator begin() { return Iterator(s_); }
  Iterator end() { return Iterator(s_ + size_); }
  const Iterator begin() const { return Iterator(s_); }
  const Iterator end() const { return Iterator(s_ + size_); }

  friend std::ostream &operator<<(std::ostream &os, const String &s) {
    return os << s.s_;
  }

  void swap(String &other) noexcept {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(s_, other.s_);
  }
};