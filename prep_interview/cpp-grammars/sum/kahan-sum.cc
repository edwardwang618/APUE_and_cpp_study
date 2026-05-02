#include <cstdio>
#include <iostream>

// 朴素求和
double naive_sum(int n, double val) {
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += val;
  }
  return sum;
}

// Kahan 补偿求和
double kahan_sum(int n, double val) {
  double sum = 0.0;
  double c = 0.0; // 补偿变量
  for (int i = 0; i < n; i++) {
    double y = val - c; // 补偿上次丢失的部分
    double t = sum + y; // 执行加法
    c = (t - sum) - y;  // 计算本次丢失的部分
    sum = t;
  }
  return sum;
}

int main() {
  int n = 10000000; // 10^7
  double val = 0.1;

  printf("Expected:  %.15f\n", 1000000.0);
  printf("Naive:     %.15f\n", naive_sum(n, val));
  printf("Kahan:     %.15f\n", kahan_sum(n, val));

  return 0;
}