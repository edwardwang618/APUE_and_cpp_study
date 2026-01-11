#include "csp_demo.h"

// Pipeline: numbers → square → print
int main() {
  Channel<int> numbers(3);
  Channel<int> squares(3);

  // Stage 1: Generate numbers
  std::thread generator([&numbers]() {
    for (int i = 1; i <= 5; i++) {
      numbers.send(i);
    }
    numbers.close();
  });

  // Stage 2: Square numbers
  std::thread squarer([&numbers, &squares]() {
    while (auto n = numbers.recv()) {
      squares.send((*n) * (*n));
    }
    squares.close();
  });

  // Stage 3: Print results
  std::thread printer([&squares]() {
    while (auto n = squares.recv()) {
      std::cout << "Result: " << *n << std::endl;
    }
  });

  generator.join();
  squarer.join();
  printer.join();

  return 0;
}