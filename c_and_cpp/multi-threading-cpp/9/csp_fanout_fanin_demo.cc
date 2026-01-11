#include "csp_demo.h"

int main() {
  Channel<int> jobs(10);
  Channel<int> results(10);

  // Fan-out: multiple workers process jobs
  std::vector<std::thread> workers;
  for (int w = 0; w < 3; w++) {
    workers.emplace_back([&jobs, &results, w]() {
      while (auto job = jobs.recv()) {
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        int result = (*job) * 2;
        std::cout << "Worker " << w << " processed " << *job << std::endl;
        results.send(result);
      }
    });
  }

  // Send jobs
  std::thread sender([&jobs]() {
    for (int i = 1; i <= 9; i++) {
      jobs.send(i);
    }
    jobs.close();
  });

  // Fan-in: collect all results
  std::thread collector([&results]() {
    int count = 0;
    while (auto r = results.recv()) {
      std::cout << "Result: " << *r << std::endl;
      if (++count == 9)
        break; // Know we have 9 jobs
    }
  });

  sender.join();
  for (auto &w : workers)
    w.join();
  results.close();
  collector.join();

  return 0;
}