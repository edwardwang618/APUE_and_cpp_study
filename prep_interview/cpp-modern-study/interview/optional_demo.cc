#include <algorithm>
#include <optional>
#include <type_traits>
#include <utility>
template <typename T> class Optional {
  bool has_value_;
  union {
    T v;
  };

public:
  Optional() : has_value_(false) {}
  ~Optional()
    requires std::is_trivially_destructible_v<T>
  = default;

  ~Optional() { reset(); }
  Optional(const Optional &other) {
    has_value_ = other.has_value_;
    if (has_value_)
      new (&v) T(other.v);
  }
  Optional &operator=(const Optional &other) {
    if (this == &other)
      return *this;
    if constexpr (std::is_copy_assignable_v<T>) {
      if (has_value_ && other.has_value_)
        v = other.v;
      else if (has_value_ && !other.has_value_) {
        v.~T();
        has_value_ = false;
      } else if (!has_value_ && other.has_value_) {
        new (&v) T(other.v);
        has_value_ = true;
      }
    } else {
      if (has_value_)
        v.~T();
      has_value_ = other.has_value_;
      if (has_value_)
        new (&v) T(other.v);
    }
    return *this;
  }
  Optional(Optional &&other) noexcept(std::is_nothrow_move_constructible_v<T>)
      : has_value_(false) {
    if (other.has_value_) {
      new (&v) T(std::move(other.v));
      has_value_ = true;
    }
  }
  Optional &operator=(Optional &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<T>) {
    if (this == &other)
      return *this;
    if constexpr (std::is_move_assignable_v<T>) {
      if (has_value_ && other.has_value_)
        v = std::move(other.v);
      else if (has_value_ && !other.has_value_) {
        v.~T();
        has_value_ = false;
      } else if (!has_value_ && other.has_value_) {
        new (&v) T(std::move(other.v));
        has_value_ = true;
      }
    } else {
      if (has_value_ && other.has_value_) {
        v.~T();
        new (&v) T(std::move(other.v));
      } else if (has_value_ && !other.has_value_) {
        v.~T();
        has_value_ = false;
      } else if (!has_value_ && other.has_value_) {
        new (&v) T(std::move(other.v));
        has_value_ = true;
      }
    }
    return *this;
  }
  Optional(const T &v) : has_value_(true), v(v) {}
  Optional(T &&v) : has_value_(true), v(std::move(v)) {}
  explicit operator bool() const noexcept { return has_value_; }
  bool has_value() const noexcept { return has_value_; }

  T &operator*() & { return v; }
  const T &operator*() const & { return v; }
  T &&operator*() && { return std::move(v); }
  const T &&operator*() const && { return std::move(v); }

  void reset() noexcept {
    if (has_value_) {
      v.~T();
      has_value_ = false;
    }
  }
  T &operator->() { return &v; }
  const T &operator->() const { return &v; }
  T &value() & {
    if (!has_value_)
      throw std::bad_optional_access{};
    return v;
  }
  const T &value() const & {
    if (!has_value_)
      throw std::bad_optional_access{};
    return v;
  }
  T &&value() && {
    if (!has_value_)
      throw std::bad_optional_access{};
    return std::move(v);
  }
  T value_or(const T &default_value) const & {
    return has_value_ ? v : default_value;
  }
  T value_or(T &&default_value) && {
    return has_value_ ? v : std::move(default_value);
  }

  template <typename... Args> T &emplace(Args &&...args) {
    reset();
    new (&v) T(std::forward<Args>(args)...);
    has_value_ = true;
    return v;
  }
};