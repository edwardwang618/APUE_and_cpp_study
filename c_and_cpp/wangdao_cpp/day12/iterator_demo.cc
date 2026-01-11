#include <iostream>

template <typename T> class ArrayIterator {
private:
  T *ptr_;

public:
  ArrayIterator(T *p) : ptr_(p) {}

  T &operator*() const { return *ptr_; }

  T *operator->() const { return ptr_; }

  // Prefix ++
  ArrayIterator &operator++() {
    ++ptr_;
    return *this;
  }

  // Postfix ++
  ArrayIterator operator++(int) {
    ArrayIterator tmp = *this;
    ++ptr_;
    return tmp;
  }

  bool operator==(const ArrayIterator &other) const {
    return ptr_ == other.ptr_;
  }

  bool operator!=(const ArrayIterator &other) const {
    return ptr_ != other.ptr_;
  }
};

// Usage
struct Point {
  int x, y;
};

int main() {
  Point points[] = {{1, 2}, {3, 4}, {5, 6}};

  ArrayIterator<Point> begin(points);
  ArrayIterator<Point> end(points + 3);

  for (auto it = begin; it != end; ++it) {
    std::cout << it->x << ", " << it->y << "\n";
    // Or: std::cout << (*it).x << ", " << (*it).y << "\n";
  }
}