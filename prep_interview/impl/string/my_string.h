#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

class String {
private:
  size_t size_;
  size_t capacity_;
  char *s_;

  void ensure_capacity(size_t required) {
    if (required <= capacity_)
      return;
    size_t new_capacity = std::max(2 * capacity_, required);
    char *p = new char[new_capacity];
    std::memcpy(p, s_, size_ + 1);
    delete[] s_;
    s_ = p;
    capacity_ = new_capacity;
  }

public:
  // Constants
  static constexpr size_t npos = static_cast<size_t>(-1);

  // ===================
  // Constructors
  // ===================

  String() : size_(0), capacity_(1), s_(new char[1]{'\0'}) {}

  String(const char *str) : size_(std::strlen(str)), capacity_(size_ + 1) {
    s_ = new char[capacity_];
    std::memcpy(s_, str, size_ + 1);
  }

  String(const char *str, size_t len) : size_(len), capacity_(len + 1) {
    s_ = new char[capacity_];
    std::memcpy(s_, str, len);
    s_[size_] = '\0';
  }

  String(size_t count, char ch) : size_(count), capacity_(count + 1) {
    s_ = new char[capacity_];
    std::fill_n(s_, count, ch);
    s_[size_] = '\0';
  }

  explicit String(char ch)
      : size_(1), capacity_(2), s_(new char[2]{ch, '\0'}) {}

  // ===================
  // Copy & Move
  // ===================

  String(const String &other) : size_(other.size_), capacity_(other.capacity_) {
    s_ = new char[capacity_];
    std::memcpy(s_, other.s_, size_ + 1);
  }

  String(String &&other) noexcept
      : size_(other.size_), capacity_(other.capacity_), s_(other.s_) {
    other.s_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  String &operator=(String other) noexcept {
    swap(other);
    return *this;
  }

  String &operator=(const char *str) {
    String tmp(str);
    swap(tmp);
    return *this;
  }

  String &operator=(char ch) {
    String tmp(ch);
    swap(tmp);
    return *this;
  }

  // ===================
  // Destructor
  // ===================

  ~String() noexcept { delete[] s_; }

  // ===================
  // Iterators
  // ===================

  template <typename CharT> class IteratorBase {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using pointer = CharT *;
    using reference = CharT &;

  private:
    pointer ptr_;

  public:
    explicit IteratorBase(pointer p = nullptr) : ptr_(p) {}

    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }
    reference operator[](difference_type n) const { return ptr_[n]; }

    IteratorBase &operator++() {
      ++ptr_;
      return *this;
    }
    IteratorBase operator++(int) {
      IteratorBase tmp(*this);
      ++ptr_;
      return tmp;
    }
    IteratorBase &operator--() {
      --ptr_;
      return *this;
    }
    IteratorBase operator--(int) {
      IteratorBase tmp(*this);
      --ptr_;
      return tmp;
    }

    IteratorBase &operator+=(difference_type n) {
      ptr_ += n;
      return *this;
    }
    IteratorBase &operator-=(difference_type n) {
      ptr_ -= n;
      return *this;
    }

    IteratorBase operator+(difference_type n) const {
      return IteratorBase(ptr_ + n);
    }
    IteratorBase operator-(difference_type n) const {
      return IteratorBase(ptr_ - n);
    }
    difference_type operator-(const IteratorBase &other) const {
      return ptr_ - other.ptr_;
    }

    friend IteratorBase operator+(difference_type n, const IteratorBase &it) {
      return IteratorBase(it.ptr_ + n);
    }

    bool operator==(const IteratorBase &other) const = default;

    std::strong_ordering operator<=>(const IteratorBase &other) const = default;

    operator IteratorBase<const char>() const {
      return IteratorBase<const char>(ptr_);
    }
  };

  using Iterator = IteratorBase<char>;
  using ConstIterator = IteratorBase<const char>;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  Iterator begin() noexcept { return Iterator(s_); }
  Iterator end() noexcept { return Iterator(s_ + size_); }
  ConstIterator begin() const noexcept { return ConstIterator(s_); }
  ConstIterator end() const noexcept { return ConstIterator(s_ + size_); }
  ConstIterator cbegin() const noexcept { return ConstIterator(s_); }
  ConstIterator cend() const noexcept { return ConstIterator(s_ + size_); }

  ReverseIterator rbegin() noexcept { return ReverseIterator(end()); }
  ReverseIterator rend() noexcept { return ReverseIterator(begin()); }
  ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator(end());
  }
  ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator(begin());
  }
  ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator(cend());
  }
  ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator(cbegin());
  }

  // ===================
  // Element Access
  // ===================

  char &operator[](size_t idx) { return s_[idx]; }
  const char &operator[](size_t idx) const { return s_[idx]; }

  char &at(size_t idx) {
    if (idx >= size_)
      throw std::out_of_range("String::at: index out of range");
    return s_[idx];
  }

  const char &at(size_t idx) const {
    if (idx >= size_)
      throw std::out_of_range("String::at: index out of range");
    return s_[idx];
  }

  char &front() { return s_[0]; }
  const char &front() const { return s_[0]; }

  char &back() { return s_[size_ - 1]; }
  const char &back() const { return s_[size_ - 1]; }

  const char *c_str() const noexcept { return s_; }
  const char *data() const noexcept { return s_; }
  char *data() noexcept { return s_; }

  // ===================
  // Capacity
  // ===================

  size_t size() const noexcept { return size_; }
  size_t length() const noexcept { return size_; }
  size_t capacity() const noexcept { return capacity_; }
  bool empty() const noexcept { return size_ == 0; }

  void reserve(size_t new_capacity) {
    if (new_capacity <= capacity_)
      return;
    char *p = new char[new_capacity];
    std::memcpy(p, s_, size_ + 1);
    delete[] s_;
    s_ = p;
    capacity_ = new_capacity;
  }

  void shrink_to_fit() {
    if (capacity_ == size_ + 1)
      return;
    char *p = new char[size_ + 1];
    std::memcpy(p, s_, size_ + 1);
    delete[] s_;
    s_ = p;
    capacity_ = size_ + 1;
  }

  // ===================
  // Modifiers
  // ===================

  void clear() noexcept {
    size_ = 0;
    s_[0] = '\0';
  }

  void push_back(char ch) {
    ensure_capacity(size_ + 2);
    s_[size_] = ch;
    s_[++size_] = '\0';
  }

  void pop_back() noexcept {
    assert(size_ > 0);
    s_[--size_] = '\0';
  }

  void resize(size_t new_size) { resize(new_size, '\0'); }

  void resize(size_t new_size, char ch) {
    if (new_size > size_) {
      ensure_capacity(new_size + 1);
      std::fill(s_ + size_, s_ + new_size, ch);
    }
    size_ = new_size;
    s_[size_] = '\0';
  }

  void swap(String &other) noexcept {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(s_, other.s_);
  }

  String &append(const String &other) {
    ensure_capacity(size_ + other.size_ + 1);
    std::memcpy(s_ + size_, other.s_, other.size_ + 1);
    size_ += other.size_;
    return *this;
  }

  String &append(const char *str) {
    size_t len = std::strlen(str);
    ensure_capacity(size_ + len + 1);
    std::memcpy(s_ + size_, str, len + 1);
    size_ += len;
    return *this;
  }

  String &append(size_t count, char ch) {
    ensure_capacity(size_ + count + 1);
    std::fill_n(s_ + size_, count, ch);
    size_ += count;
    s_[size_] = '\0';
    return *this;
  }

  String &insert(size_t pos, const String &other) {
    if (pos > size_)
      throw std::out_of_range("String::insert: position out of range");
    ensure_capacity(size_ + other.size_ + 1);
    std::memmove(s_ + pos + other.size_, s_ + pos, size_ - pos + 1);
    std::memcpy(s_ + pos, other.s_, other.size_);
    size_ += other.size_;
    return *this;
  }

  String &insert(size_t pos, const char *str) {
    if (pos > size_)
      throw std::out_of_range("String::insert: position out of range");
    size_t len = std::strlen(str);
    ensure_capacity(size_ + len + 1);
    std::memmove(s_ + pos + len, s_ + pos, size_ - pos + 1);
    std::memcpy(s_ + pos, str, len);
    size_ += len;
    return *this;
  }

  String &insert(size_t pos, size_t count, char ch) {
    if (pos > size_)
      throw std::out_of_range("String::insert: position out of range");
    ensure_capacity(size_ + count + 1);
    std::memmove(s_ + pos + count, s_ + pos, size_ - pos + 1);
    std::fill_n(s_ + pos, count, ch);
    size_ += count;
    return *this;
  }

  String &erase(size_t pos = 0, size_t count = npos) {
    if (pos > size_)
      throw std::out_of_range("String::erase: position out of range");
    count = std::min(count, size_ - pos);
    std::memmove(s_ + pos, s_ + pos + count, size_ - pos - count + 1);
    size_ -= count;
    return *this;
  }

  String &replace(size_t pos, size_t count, const String &other) {
    if (pos > size_)
      throw std::out_of_range("String::replace: position out of range");
    count = std::min(count, size_ - pos);
    size_t new_size = size_ - count + other.size_;
    ensure_capacity(new_size + 1);
    std::memmove(s_ + pos + other.size_, s_ + pos + count,
                 size_ - pos - count + 1);
    std::memcpy(s_ + pos, other.s_, other.size_);
    size_ = new_size;
    return *this;
  }

  String &replace(size_t pos, size_t count, const char *str) {
    return replace(pos, count, String(str));
  }

  // ===================
  // Compound Assignment
  // ===================

  String &operator+=(const String &other) { return append(other); }
  String &operator+=(const char *str) { return append(str); }
  String &operator+=(char ch) {
    push_back(ch);
    return *this;
  }

  // ===================
  // String Operations
  // ===================

  String substr(size_t pos = 0, size_t count = npos) const {
    if (pos > size_)
      throw std::out_of_range("String::substr: position out of range");
    count = std::min(count, size_ - pos);
    return String(s_ + pos, count);
  }

  int compare(const String &other) const noexcept {
    return std::strcmp(s_, other.s_);
  }

  int compare(const char *str) const noexcept { return std::strcmp(s_, str); }

  bool starts_with(const String &prefix) const noexcept {
    if (prefix.size_ > size_)
      return false;
    return std::memcmp(s_, prefix.s_, prefix.size_) == 0;
  }

  bool starts_with(const char *prefix) const noexcept {
    size_t len = std::strlen(prefix);
    if (len > size_)
      return false;
    return std::memcmp(s_, prefix, len) == 0;
  }

  bool starts_with(char ch) const noexcept { return size_ > 0 && s_[0] == ch; }

  bool ends_with(const String &suffix) const noexcept {
    if (suffix.size_ > size_)
      return false;
    return std::memcmp(s_ + size_ - suffix.size_, suffix.s_, suffix.size_) == 0;
  }

  bool ends_with(const char *suffix) const noexcept {
    size_t len = std::strlen(suffix);
    if (len > size_)
      return false;
    return std::memcmp(s_ + size_ - len, suffix, len) == 0;
  }

  bool ends_with(char ch) const noexcept {
    return size_ > 0 && s_[size_ - 1] == ch;
  }

  bool contains(const String &str) const noexcept { return find(str) != npos; }

  bool contains(const char *str) const noexcept { return find(str) != npos; }

  bool contains(char ch) const noexcept { return find(ch) != npos; }

  // ===================
  // Find Operations
  // ===================

  size_t find(char ch, size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i)
      if (s_[i] == ch)
        return i;
    return npos;
  }

  size_t find(const String &other, size_t pos = 0) const noexcept {
    if (other.size_ == 0)
      return pos <= size_ ? pos : npos;
    if (other.size_ > size_)
      return npos;
    for (size_t i = pos; i + other.size_ <= size_; ++i)
      if (std::memcmp(s_ + i, other.s_, other.size_) == 0)
        return i;
    return npos;
  }

  size_t find(const char *str, size_t pos = 0) const noexcept {
    return find(String(str), pos);
  }

  size_t rfind(char ch, size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i)
      if (s_[i - 1] == ch)
        return i - 1;
    return npos;
  }

  size_t rfind(const String &other, size_t pos = npos) const noexcept {
    if (other.size_ == 0)
      return std::min(pos, size_);
    if (other.size_ > size_)
      return npos;
    pos = std::min(pos, size_ - other.size_);
    for (size_t i = pos + 1; i > 0; --i)
      if (std::memcmp(s_ + i - 1, other.s_, other.size_) == 0)
        return i - 1;
    return npos;
  }

  size_t find_first_of(const String &chars, size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i)
      for (size_t j = 0; j < chars.size_; ++j)
        if (s_[i] == chars.s_[j])
          return i;
    return npos;
  }

  size_t find_first_of(const char *chars, size_t pos = 0) const noexcept {
    return find_first_of(String(chars), pos);
  }

  size_t find_last_of(const String &chars, size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i)
      for (size_t j = 0; j < chars.size_; ++j)
        if (s_[i - 1] == chars.s_[j])
          return i - 1;
    return npos;
  }

  size_t find_first_not_of(const String &chars, size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i) {
      bool found = false;
      for (size_t j = 0; j < chars.size_ && !found; ++j)
        if (s_[i] == chars.s_[j])
          found = true;
      if (!found)
        return i;
    }
    return npos;
  }

  size_t find_last_not_of(const String &chars,
                          size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i) {
      bool found = false;
      for (size_t j = 0; j < chars.size_ && !found; ++j)
        if (s_[i - 1] == chars.s_[j])
          found = true;
      if (!found)
        return i - 1;
    }
    return npos;
  }

  // ===================
  // Comparison Operators
  // ===================

  bool operator==(const String &other) const noexcept {
    return size_ == other.size_ && compare(other) == 0;
  }

  std::strong_ordering operator<=>(const String &other) const noexcept {
    return compare(other) <=> 0;
  }

  bool operator==(const char *str) const noexcept { return compare(str) == 0; }

  std::strong_ordering operator<=>(const char *str) const noexcept {
    return compare(str) <=> 0;
  }

  // ===================
  // Concatenation
  // ===================

  friend String operator+(const String &lhs, const String &rhs) {
    String result;
    result.reserve(lhs.size_ + rhs.size_ + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  friend String operator+(const String &lhs, const char *rhs) {
    String result;
    size_t rhs_len = std::strlen(rhs);
    result.reserve(lhs.size_ + rhs_len + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  friend String operator+(const char *lhs, const String &rhs) {
    String result;
    size_t lhs_len = std::strlen(lhs);
    result.reserve(lhs_len + rhs.size_ + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  friend String operator+(const String &lhs, char rhs) {
    String result;
    result.reserve(lhs.size_ + 2);
    result.append(lhs);
    result.push_back(rhs);
    return result;
  }

  friend String operator+(char lhs, const String &rhs) {
    String result;
    result.reserve(rhs.size_ + 2);
    result.push_back(lhs);
    result.append(rhs);
    return result;
  }

  // Move-optimized concatenation
  friend String operator+(String &&lhs, const String &rhs) {
    lhs.append(rhs);
    return std::move(lhs);
  }

  friend String operator+(String &&lhs, const char *rhs) {
    lhs.append(rhs);
    return std::move(lhs);
  }

  friend String operator+(String &&lhs, char rhs) {
    lhs.push_back(rhs);
    return std::move(lhs);
  }

  // ===================
  // Stream Operators
  // ===================

  friend std::ostream &operator<<(std::ostream &os, const String &str) {
    return os << str.s_;
  }

  friend std::istream &operator>>(std::istream &is, String &str) {
    str.clear();
    is >> std::ws;
    char ch;
    while (is.get(ch) && !std::isspace(static_cast<unsigned char>(ch)))
      str.push_back(ch);
    return is;
  }

  // ===================
  // Free Functions
  // ===================

  friend void swap(String &a, String &b) noexcept { a.swap(b); }
};

// getline function
inline std::istream &getline(std::istream &is, String &str, char delim = '\n') {
  str.clear();
  char ch;
  while (is.get(ch) && ch != delim)
    str.push_back(ch);
  return is;
}

// Hash support for use in unordered containers
namespace std {
template <> struct hash<String> {
  size_t operator()(const String &s) const noexcept {
    size_t h = 0;
    for (size_t i = 0; i < s.size(); ++i)
      h = h * 31 + static_cast<unsigned char>(s[i]);
    return h;
  }
};
} // namespace std