#pragma once
#include <iostream>
#include <type_traits>

// 1. Basic traits example - identify container type
template <typename T> struct ContainerTraits {
  static constexpr bool is_array = false;
  static constexpr bool is_pointer = false;
  using value_type = T;
};

// Specialization for arrays
template <typename T, size_t N> struct ContainerTraits<T[N]> {
  static constexpr bool is_array = true;
  static constexpr bool is_pointer = false;
  static constexpr size_t size = N;
  using value_type = T;
};

// Specialization for pointers
template <typename T> struct ContainerTraits<T *> {
  static constexpr bool is_array = false;
  static constexpr bool is_pointer = true;
  using value_type = T;
};

// 2. Behavior modification traits
template <typename T> struct PrintTraits {
  static void print(const T &value) { std::cout << value; }
};

// Specialization for pointers
template <typename T> struct PrintTraits<T *> {
  static void print(T *value) {
    if (value) {
      std::cout << *value;
    } else {
      std::cout << "nullptr";
    }
  }
};

// 3. Policy traits - different comparison behaviors
template <typename T> struct DefaultCompare {
  static bool less(const T &a, const T &b) { return a < b; }
};

template <typename T> struct CaseInsensitiveCompare {
  static bool less(const T &a, const T &b) {
    return std::tolower(a) < std::tolower(b);
  }
};

// 4. Type modification traits
template <typename T> struct MakeConst {
  using type = const T;
};

// 5. Container that uses traits
template <typename T, typename ComparePolicy = DefaultCompare<T>>
class SortableContainer {
public:
  SortableContainer(T value) : data(value) {}

  bool isLessThan(const SortableContainer &other) const {
    return ComparePolicy::less(data, other.data);
  }

  // Use print traits
  void print() const {
    PrintTraits<T>::print(data);
    std::cout << std::endl;
  }

  // Use container traits
  void showTypeInfo() const {
    std::cout << "Type info:\n"
              << "  Is array: " << std::boolalpha
              << ContainerTraits<T>::is_array << "\n"
              << "  Is pointer: " << ContainerTraits<T>::is_pointer << "\n";
  }

private:
  T data;
};

// 6. SFINAE with traits
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
safe_divide(T a, T b) {
  if (b == 0)
    throw std::runtime_error("Division by zero");
  return a / b;
}

// 7. Tag dispatch example
struct pod_tag {};
struct non_pod_tag {};

template <typename T> void initialize_impl(T &value, pod_tag) {
  std::memset(&value, 0, sizeof(T));
}

template <typename T> void initialize_impl(T &value, non_pod_tag) {
  value = T();
}

template <typename T> void initialize(T &value) {
  using tag = typename std::conditional<std::is_pod<T>::value, pod_tag,
                                        non_pod_tag>::type;
  initialize_impl(value, tag());
}
