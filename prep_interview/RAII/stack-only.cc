#include <chrono>
#include <iostream>
#include <string>
#include <vector>

class Timer {
  std::chrono::high_resolution_clock::time_point start;
  const char *name;

public:
  Timer(const char *n)
      : name(n), start(std::chrono::high_resolution_clock::now()) {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << ": " << ms.count() << "ms\n";
  }

  // 禁止拷贝
  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;

  // 禁止堆分配
  void *operator new(size_t) = delete;
  void *operator new[](size_t) = delete;
};

int main() {
  {
    const int N = 1000000;
    std::string name = "std::vector push " + std::to_string(N) + " times";
    Timer time(name.c_str());
    std::vector<int> v;
    for (int i = 0; i < N; i++)
      v.push_back(i);
  }
}