#include <chrono>
#include <iostream>
#include <vector>

// Compiler will auto-vectorize this with -O3
void add_arrays(float *a, float *b, float *c, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
  }
}

// Prevent auto-vectorization
void add_arrays_scalar(float *a, float *b, float *c, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    c[i] = a[i] + b[i];
    asm volatile("" ::: "memory"); // Prevent optimization
  }
}

int main() {
  const size_t N = 100000000;

  std::vector<float> a(N, 1.0f);
  std::vector<float> b(N, 2.0f);
  std::vector<float> c(N);

  // SIMD (auto-vectorized)
  auto start1 = std::chrono::high_resolution_clock::now();
  add_arrays(a.data(), b.data(), c.data(), N);
  auto end1 = std::chrono::high_resolution_clock::now();
  auto time1 = std::chrono::duration<double, std::milli>(end1 - start1).count();

  // Scalar (no SIMD)
  auto start2 = std::chrono::high_resolution_clock::now();
  add_arrays_scalar(a.data(), b.data(), c.data(), N);
  auto end2 = std::chrono::high_resolution_clock::now();
  auto time2 = std::chrono::duration<double, std::milli>(end2 - start2).count();

  std::cout << "SIMD:   " << time1 << " ms" << std::endl;
  std::cout << "Scalar: " << time2 << " ms" << std::endl;
  std::cout << "Speedup: " << time2 / time1 << "x" << std::endl;

  return 0;
}