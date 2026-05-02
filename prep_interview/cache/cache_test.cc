// cache_test.cpp
#include <chrono>
#include <iostream>
#include <vector>

// 顺序访问，cache 友好
void sequential(std::vector<int> &v) {
  for (int i = 0; i < v.size(); i++) {
    v[i] *= 2;
  }
}

// 随机跳跃访问，cache 不友好
void strided(std::vector<int> &v) {
  for (int i = 0; i < v.size(); i++) {
    v[(i * 1009) % v.size()] *= 2; // 大步长跳跃
  }
}

int main() {
  std::vector<int> v(10000000, 1); // 10M 个 int

  sequential(v);
  strided(v);

  return 0;
}