#include <chrono>
#include <cstdint>
#include <iostream>

struct Aligned {
  double a;
  double b;
  double c;
  double d;
};

#pragma pack(push, 1)
struct Packed {
  char pad;
  double a;
  double b;
  double c;
  double d;
};
#pragma pack(pop)

// Prevent inlining so compiler can't optimize too aggressively
__attribute__((noinline)) double sum_aligned(Aligned *arr, int N) {
  double sum = 0;
  for (int i = 0; i < N; i++) {
    sum += arr[i].a;
    sum += arr[i].b;
    sum += arr[i].c;
    sum += arr[i].d;
  }
  return sum;
}

__attribute__((noinline)) double sum_packed(Packed *arr, int N) {
  double sum = 0;
  for (int i = 0; i < N; i++) {
    sum += arr[i].a;
    sum += arr[i].b;
    sum += arr[i].c;
    sum += arr[i].d;
  }
  return sum;
}

int main() {
  const int N = 2'000'000;
  const int ITER = 50;

  std::cout << "sizeof(Aligned): " << sizeof(Aligned) << "\n";
  std::cout << "sizeof(Packed):  " << sizeof(Packed) << "\n\n";

  Aligned *a = new Aligned[N];
  Packed *p = new Packed[N];

  for (int i = 0; i < N; i++) {
    a[i] = {1.0, 2.0, 3.0, 4.0};
    p[i] = {0, 1.0, 2.0, 3.0, 4.0};
  }

  volatile double sink;

  // Warm up
  sink = sum_aligned(a, N);
  sink = sum_packed(p, N);

  // Benchmark aligned
  auto t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < ITER; i++) {
    sink = sum_aligned(a, N);
  }
  auto t2 = std::chrono::high_resolution_clock::now();

  // Benchmark packed
  for (int i = 0; i < ITER; i++) {
    sink = sum_packed(p, N);
  }
  auto t3 = std::chrono::high_resolution_clock::now();

  auto ms1 =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  auto ms2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

  std::cout << "Aligned: " << ms1 << " ms\n";
  std::cout << "Packed:  " << ms2 << " ms\n";
  std::cout << "Ratio:   " << (double)ms2 / ms1 << "x\n";

  delete[] a;
  delete[] p;

  return 0;
}