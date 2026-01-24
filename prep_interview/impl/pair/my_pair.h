#include <concepts>
#include <type_traits>
#include <utility>

template <typename T1, typename T2> struct Pair {
  T1 first{};
  T2 second{};

  // Constructors
  constexpr Pair()
    requires(std::default_initializable<T1> && std::default_initializable<T2>)
  = default; // Default

  template <typename U1, typename U2>
  constexpr Pair(U1 &&x, U2 &&y)
      : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {} // Forwarding

  Pair(const Pair &) = default; // Copy
  Pair(Pair &&) = default;      // Move

  template <typename U1, typename U2>
    requires(!std::same_as<Pair<U1, U2>, Pair<T1, T2>>)
  constexpr Pair(const Pair<U1, U2> &other)
      : first(other.first), second(other.second) {} // Converting copy

  template <typename U1, typename U2>
    requires(!std::same_as<Pair<U1, U2>, Pair<T1, T2>>)
  constexpr Pair(Pair<U1, U2> &&other)
      : first(std::move(other.first)), second(std::move(other.second)) {
  } // Converting move

  // Assignment
  Pair &operator=(const Pair &other) = default;
  Pair &operator=(Pair &&other) noexcept = default;

  template <typename U1, typename U2>
    requires(!std::same_as<Pair<U1, U2>, Pair<T1, T2>>)
  Pair &operator=(const Pair<U1, U2> &other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  template <typename U1, typename U2>
    requires(!std::same_as<Pair<U1, U2>, Pair<T1, T2>>)
  Pair &operator=(Pair<U1, U2> &&other) {
    first = std::move(other.first);
    second = std::move(other.second);
    return *this;
  }

  // Swap
  void swap(Pair &other) noexcept {
    using std::swap;
    swap(first, other.first);
    swap(second, other.second);
  }

  // Comparison (C++20)
  bool operator==(const Pair &other) const = default;
  auto operator<=>(const Pair &other) const = default;
};

// Helper functions
template <typename T1, typename T2>
constexpr Pair<std::decay_t<T1>, std::decay_t<T2>> make_pair(T1 &&x, T2 &&y) {
  return Pair<std::decay_t<T1>, std::decay_t<T2>>(std::forward<T1>(x),
                                                  std::forward<T2>(y));
}

template <typename T1, typename T2>
void swap(Pair<T1, T2> &a, Pair<T1, T2> &b) noexcept {
  a.swap(b);
}