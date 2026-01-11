#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string_view>
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
    std::ranges::fill_n(s_, count, ch);
    s_[size_] = '\0';
  }

  explicit String(char ch)
      : size_(1), capacity_(2), s_(new char[2]{ch, '\0'}) {}

  // Range constructor (C++20)
  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, char>
  explicit String(R &&range) : size_(0), capacity_(1), s_(new char[1]{'\0'}) {
    if constexpr (std::ranges::sized_range<R>) {
      reserve(std::ranges::size(range) + 1);
    }
    for (char ch : range) {
      push_back(ch);
    }
  }

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
  // Iterators (C++20 style with contiguous_iterator)
  // ===================

  using Iterator = char *;
  using ConstIterator = const char *;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  // Standard library style aliases
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using reverse_iterator = ReverseIterator;
  using const_reverse_iterator = ConstReverseIterator;
  using value_type = char;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = char &;
  using const_reference = const char &;
  using pointer = char *;
  using const_pointer = const char *;

  constexpr Iterator begin() noexcept { return s_; }
  constexpr Iterator end() noexcept { return s_ + size_; }
  constexpr ConstIterator begin() const noexcept { return s_; }
  constexpr ConstIterator end() const noexcept { return s_ + size_; }
  constexpr ConstIterator cbegin() const noexcept { return s_; }
  constexpr ConstIterator cend() const noexcept { return s_ + size_; }

  constexpr ReverseIterator rbegin() noexcept { return ReverseIterator(end()); }
  constexpr ReverseIterator rend() noexcept { return ReverseIterator(begin()); }
  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator(end());
  }
  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator(begin());
  }
  constexpr ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator(cend());
  }
  constexpr ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator(cbegin());
  }

  // ===================
  // Element Access
  // ===================

  constexpr char &operator[](size_t idx) { return s_[idx]; }
  constexpr const char &operator[](size_t idx) const { return s_[idx]; }

  constexpr char &at(size_t idx) {
    if (idx >= size_) [[unlikely]]
      throw std::out_of_range("String::at: index out of range");
    return s_[idx];
  }

  constexpr const char &at(size_t idx) const {
    if (idx >= size_) [[unlikely]]
      throw std::out_of_range("String::at: index out of range");
    return s_[idx];
  }

  constexpr char &front() { return s_[0]; }
  constexpr const char &front() const { return s_[0]; }

  constexpr char &back() { return s_[size_ - 1]; }
  constexpr const char &back() const { return s_[size_ - 1]; }

  constexpr const char *c_str() const noexcept { return s_; }
  constexpr const char *data() const noexcept { return s_; }
  constexpr char *data() noexcept { return s_; }

  // ===================
  // Capacity
  // ===================

  [[nodiscard]] constexpr size_t size() const noexcept { return size_; }
  [[nodiscard]] constexpr size_t length() const noexcept { return size_; }
  [[nodiscard]] constexpr size_t capacity() const noexcept { return capacity_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

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

  constexpr void clear() noexcept {
    size_ = 0;
    s_[0] = '\0';
  }

  void push_back(char ch) {
    ensure_capacity(size_ + 2);
    s_[size_] = ch;
    s_[++size_] = '\0';
  }

  constexpr void pop_back() noexcept {
    assert(size_ > 0);
    s_[--size_] = '\0';
  }

  void resize(size_t new_size) { resize(new_size, '\0'); }

  void resize(size_t new_size, char ch) {
    if (new_size > size_) {
      ensure_capacity(new_size + 1);
      std::ranges::fill(s_ + size_, s_ + new_size, ch);
    }
    size_ = new_size;
    s_[size_] = '\0';
  }

  constexpr void swap(String &other) noexcept {
    std::ranges::swap(size_, other.size_);
    std::ranges::swap(capacity_, other.capacity_);
    std::ranges::swap(s_, other.s_);
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
    std::ranges::fill_n(s_ + size_, count, ch);
    size_ += count;
    s_[size_] = '\0';
    return *this;
  }

  // Range append (C++20)
  template <std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, char>
  String &append_range(R &&range) {
    if constexpr (std::ranges::sized_range<R>) {
      ensure_capacity(size_ + std::ranges::size(range) + 1);
    }
    for (char ch : range) {
      push_back(ch);
    }
    return *this;
  }

  String &insert(size_t pos, const String &other) {
    if (pos > size_) [[unlikely]]
      throw std::out_of_range("String::insert: position out of range");
    ensure_capacity(size_ + other.size_ + 1);
    std::memmove(s_ + pos + other.size_, s_ + pos, size_ - pos + 1);
    std::memcpy(s_ + pos, other.s_, other.size_);
    size_ += other.size_;
    return *this;
  }

  String &insert(size_t pos, const char *str) {
    if (pos > size_) [[unlikely]]
      throw std::out_of_range("String::insert: position out of range");
    size_t len = std::strlen(str);
    ensure_capacity(size_ + len + 1);
    std::memmove(s_ + pos + len, s_ + pos, size_ - pos + 1);
    std::memcpy(s_ + pos, str, len);
    size_ += len;
    return *this;
  }

  String &insert(size_t pos, size_t count, char ch) {
    if (pos > size_) [[unlikely]]
      throw std::out_of_range("String::insert: position out of range");
    ensure_capacity(size_ + count + 1);
    std::memmove(s_ + pos + count, s_ + pos, size_ - pos + 1);
    std::ranges::fill_n(s_ + pos, count, ch);
    size_ += count;
    return *this;
  }

  String &erase(size_t pos = 0, size_t count = npos) {
    if (pos > size_) [[unlikely]]
      throw std::out_of_range("String::erase: position out of range");
    count = std::min(count, size_ - pos);
    std::memmove(s_ + pos, s_ + pos + count, size_ - pos - count + 1);
    size_ -= count;
    return *this;
  }

  String &replace(size_t pos, size_t count, const String &other) {
    if (pos > size_) [[unlikely]]
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

  [[nodiscard]] String substr(size_t pos = 0, size_t count = npos) const {
    if (pos > size_) [[unlikely]]
      throw std::out_of_range("String::substr: position out of range");
    count = std::min(count, size_ - pos);
    return String(s_ + pos, count);
  }

  [[nodiscard]] constexpr int compare(const String &other) const noexcept {
    return std::string_view(s_, size_).compare(
        std::string_view(other.s_, other.size_));
  }

  [[nodiscard]] constexpr int compare(const char *str) const noexcept {
    return std::string_view(s_, size_).compare(str);
  }

  [[nodiscard]] constexpr bool starts_with(std::string_view sv) const noexcept {
    if (sv.size() > size_)
      return false;
    return std::memcmp(s_, sv.data(), sv.size()) == 0;
  }

  [[nodiscard]] constexpr bool starts_with(char ch) const noexcept {
    return size_ > 0 && s_[0] == ch;
  }

  [[nodiscard]] constexpr bool ends_with(std::string_view sv) const noexcept {
    if (sv.size() > size_)
      return false;
    return std::memcmp(s_ + size_ - sv.size(), sv.data(), sv.size()) == 0;
  }

  [[nodiscard]] constexpr bool ends_with(char ch) const noexcept {
    return size_ > 0 && s_[size_ - 1] == ch;
  }

  [[nodiscard]] constexpr bool contains(std::string_view sv) const noexcept {
    return find(sv) != npos;
  }

  [[nodiscard]] constexpr bool contains(char ch) const noexcept {
    return find(ch) != npos;
  }

  // Implicit conversion to string_view (C++20 friendly)
  [[nodiscard]] constexpr operator std::string_view() const noexcept {
    return std::string_view(s_, size_);
  }

  // ===================
  // Find Operations
  // ===================

  [[nodiscard]] constexpr size_t find(char ch, size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i)
      if (s_[i] == ch)
        return i;
    return npos;
  }

  [[nodiscard]] constexpr size_t find(std::string_view sv,
                                      size_t pos = 0) const noexcept {
    if (sv.empty())
      return pos <= size_ ? pos : npos;
    if (sv.size() > size_)
      return npos;
    for (size_t i = pos; i + sv.size() <= size_; ++i)
      if (std::memcmp(s_ + i, sv.data(), sv.size()) == 0)
        return i;
    return npos;
  }

  [[nodiscard]] constexpr size_t rfind(char ch,
                                       size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i)
      if (s_[i - 1] == ch)
        return i - 1;
    return npos;
  }

  [[nodiscard]] constexpr size_t rfind(std::string_view sv,
                                       size_t pos = npos) const noexcept {
    if (sv.empty())
      return std::min(pos, size_);
    if (sv.size() > size_)
      return npos;
    pos = std::min(pos, size_ - sv.size());
    for (size_t i = pos + 1; i > 0; --i)
      if (std::memcmp(s_ + i - 1, sv.data(), sv.size()) == 0)
        return i - 1;
    return npos;
  }

  [[nodiscard]] constexpr size_t find_first_of(std::string_view chars,
                                               size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i)
      if (chars.find(s_[i]) != std::string_view::npos)
        return i;
    return npos;
  }

  [[nodiscard]] constexpr size_t
  find_last_of(std::string_view chars, size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i)
      if (chars.find(s_[i - 1]) != std::string_view::npos)
        return i - 1;
    return npos;
  }

  [[nodiscard]] constexpr size_t
  find_first_not_of(std::string_view chars, size_t pos = 0) const noexcept {
    for (size_t i = pos; i < size_; ++i)
      if (chars.find(s_[i]) == std::string_view::npos)
        return i;
    return npos;
  }

  [[nodiscard]] constexpr size_t
  find_last_not_of(std::string_view chars, size_t pos = npos) const noexcept {
    if (size_ == 0)
      return npos;
    pos = std::min(pos, size_ - 1);
    for (size_t i = pos + 1; i > 0; --i)
      if (chars.find(s_[i - 1]) == std::string_view::npos)
        return i - 1;
    return npos;
  }

  // ===================
  // Three-Way Comparison (C++20 Spaceship Operator)
  // ===================

  [[nodiscard]] constexpr std::strong_ordering
  operator<=>(const String &other) const noexcept {
    return std::string_view(s_, size_) <=>
           std::string_view(other.s_, other.size_);
  }

  [[nodiscard]] constexpr std::strong_ordering
  operator<=>(const char *str) const noexcept {
    return std::string_view(s_, size_) <=> std::string_view(str);
  }

  [[nodiscard]] constexpr std::strong_ordering
  operator<=>(std::string_view sv) const noexcept {
    int result = std::string_view(s_, size_).compare(sv);
    if (result < 0)
      return std::strong_ordering::less;
    if (result > 0)
      return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }

  // Equality (needed separately for heterogeneous comparison)
  [[nodiscard]] constexpr bool operator==(const String &other) const noexcept {
    return size_ == other.size_ && std::memcmp(s_, other.s_, size_) == 0;
  }

  [[nodiscard]] constexpr bool operator==(const char *str) const noexcept {
    return std::string_view(s_, size_) == std::string_view(str);
  }

  [[nodiscard]] constexpr bool operator==(std::string_view sv) const noexcept {
    return size_ == sv.size() && std::memcmp(s_, sv.data(), size_) == 0;
  }

  // ===================
  // Concatenation
  // ===================

  [[nodiscard]] friend String operator+(const String &lhs, const String &rhs) {
    String result;
    result.reserve(lhs.size_ + rhs.size_ + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  [[nodiscard]] friend String operator+(const String &lhs, const char *rhs) {
    String result;
    size_t rhs_len = std::strlen(rhs);
    result.reserve(lhs.size_ + rhs_len + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  [[nodiscard]] friend String operator+(const char *lhs, const String &rhs) {
    String result;
    size_t lhs_len = std::strlen(lhs);
    result.reserve(lhs_len + rhs.size_ + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
  }

  [[nodiscard]] friend String operator+(const String &lhs, char rhs) {
    String result;
    result.reserve(lhs.size_ + 2);
    result.append(lhs);
    result.push_back(rhs);
    return result;
  }

  [[nodiscard]] friend String operator+(char lhs, const String &rhs) {
    String result;
    result.reserve(rhs.size_ + 2);
    result.push_back(lhs);
    result.append(rhs);
    return result;
  }

  // Move-optimized concatenation
  [[nodiscard]] friend String operator+(String &&lhs, const String &rhs) {
    lhs.append(rhs);
    return std::move(lhs);
  }

  [[nodiscard]] friend String operator+(String &&lhs, const char *rhs) {
    lhs.append(rhs);
    return std::move(lhs);
  }

  [[nodiscard]] friend String operator+(String &&lhs, char rhs) {
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

  friend constexpr void swap(String &a, String &b) noexcept { a.swap(b); }
};

// getline function
inline std::istream &getline(std::istream &is, String &str, char delim = '\n') {
  str.clear();
  char ch;
  while (is.get(ch) && ch != delim)
    str.push_back(ch);
  return is;
}

// Hash support (C++20 style)
template <> struct std::hash<String> {
  [[nodiscard]] size_t operator()(const String &s) const noexcept {
    return std::hash<std::string_view>{}(std::string_view(s));
  }
};

// Formatter support (C++20)
#if __has_include(<format>)
#include <format>
template <> struct std::formatter<String> : std::formatter<std::string_view> {
  auto format(const String &s, std::format_context &ctx) const {
    return std::formatter<std::string_view>::format(std::string_view(s), ctx);
  }
};
#endif