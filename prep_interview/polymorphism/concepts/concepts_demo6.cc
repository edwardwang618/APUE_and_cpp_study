#include <concepts>
#include <iostream>
#include <list>
#include <vector>

// 随机访问迭代器
template <typename Iter>
concept RandomAccess = requires(Iter it, int n) {
  { it + n } -> std::same_as<Iter>;
  { it - n } -> std::same_as<Iter>;
  { it[n] };
};

// 随机访问：用快速二分查找
template <RandomAccess Iter> bool contains(Iter begin, Iter end, int value) {
  std::cout << "[using binary search] ";
  while (begin < end) {
    auto mid = begin + (end - begin) / 2;
    if (*mid == value)
      return true;
    if (*mid < value)
      begin = mid + 1;
    else
      end = mid;
  }
  return false;
}

// 其他迭代器：用线性查找
template <typename Iter> bool contains(Iter begin, Iter end, int value) {
  std::cout << "[using linear search] ";
  while (begin != end) {
    if (*begin == value)
      return true;
    ++begin;
  }
  return false;
}

int main() {
  std::vector<int> vec{1, 2, 3, 4, 5};
  std::list<int> lst{1, 2, 3, 4, 5};

  // vector 有随机访问，用二分
  std::cout << contains(vec.begin(), vec.end(), 3) << "\n";

  // list 没有随机访问，用线性
  std::cout << contains(lst.begin(), lst.end(), 3) << "\n";
}