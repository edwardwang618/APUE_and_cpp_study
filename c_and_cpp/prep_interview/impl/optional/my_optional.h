#include <optional>
#include <type_traits>
#include <utility>

template <typename T> class Optional {
public:
  Optional() : has_value_(false) {}
  Optional(const T &value) : value_(value), has_value_(true) {}
  Optional(T &&value) : value_(std::move(value)), has_value_(true) {}
  Optional(std::nullopt_t) : has_value_(false) {}

  Optional(const Optional &other) : has_value_(other.has_value_) {
    if (has_value_)
      new (&value_) T(other.value_);
  }
  Optional(Optional &&other) : has_value_(other.has_value_) {
    if (has_value_)
      new (&value_) T(std::move(other.value_));
  }
  Optional &operator=(const Optional &other) {
    if (this != &other) {
      if (has_value_ && other.has_value_) {
        if constexpr (std::is_copy_assignable_v<T>)
          value_ = other.value_;
        else {
          value_.~T();
          new (&value_) T(other.value_);
        }
      } else if (has_value_) {
        value_.~T();
        has_value_ = false;
      } else if (other.has_value_) {
        new (&value_) T(other.value_);
        has_value_ = true;
      }
    }
    return *this;
  }
  Optional &operator=(Optional &&other) {
    if (this != &other) {
      if (has_value_ && other.has_value_) {
        if constexpr (std::is_move_assignable_v<T>)
          value_ = std::move(other.value_);
        else {
          value_.~T();
          new (&value_) T(std::move(other.value_));
        }
      } else if (has_value_) {
        value_.~T();
        has_value_ = false;
      } else if (other.has_value_) {
        new (&value_) T(std::move(other.value_));
        has_value_ = true;
      }
    }
    return *this;
  }

  Optional &operator=(std::nullopt_t) {
    if (has_value_) {
      value_.~T();
      has_value_ = false;
    }
    return *this;
  }

  ~Optional() {
    if (has_value_)
      value_.~T();
  }

  bool has_value() const { return has_value_; }
  T &value() {
    if (!has_value_)
      throw std::bad_optional_access();
    return value_;
  }
  const T &value() const {
    if (!has_value_)
      throw std::bad_optional_access();
    return value_;
  }

  T value_or(const T &default_value) const {
    return has_value_ ? value_ : default_value;
  }
  T value_or(T &&default_value) const {
    return has_value_ ? value_ : std::move(default_value);
  }

  T &operator*() { return value_; }
  const T &operator*() const { return value_; }

  T *operator->() { return &value_; }
  const T *operator->() const { return &value_; }

  operator bool() const { return has_value_; }

  bool operator==(const Optional &other) const {
    if (has_value_ && other.has_value_)
      return value_ == other.value_;
    return !has_value_ && !other.has_value_;
  }

  bool operator!=(const Optional &other) const { return !operator==(other); }

  bool operator==(const T &value) const {
    return has_value_ && value_ == value;
  }
  bool operator!=(const T &value) const { return !operator==(value); }

  bool operator==(std::nullopt_t) const { return !has_value_; }
  bool operator!=(std::nullopt_t) const { return has_value_; }

  void reset() {
    if (has_value_)
      value_.~T();
    has_value_ = false;
  }

  template <typename... Args> void emplace(Args &&...args) {
    if (has_value_)
      value_.~T();
    new (&value_) T(std::forward<Args>(args)...);
    has_value_ = true;
  }

private:
  union {
    T value_;
  };
  bool has_value_;
};