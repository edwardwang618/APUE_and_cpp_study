#include <chrono>
#include <iostream>

constexpr int SIZE = 4096;
int src[SIZE][SIZE], dst[SIZE][SIZE];

void copyij() {
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j) dst[i][j] = src[i][j];
}

void copyji() {
  for (int j = 0; j < SIZE; ++j)
    for (int i = 0; i < SIZE; ++i) dst[i][j] = src[i][j];
}

int main() {
  // Initialize src array
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j) src[i][j] = i + j;

  // Measure time for copyij
  auto start = std::chrono::high_resolution_clock::now();
  copyij();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_copyij = end - start;
  std::cout << "Time for copyij: " << elapsed_copyij.count() << " seconds\n";

  // Measure time for copyji
  start = std::chrono::high_resolution_clock::now();
  copyji();
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_copyji = end - start;
  std::cout << "Time for copyji: " << elapsed_copyji.count() << " seconds\n";
}
