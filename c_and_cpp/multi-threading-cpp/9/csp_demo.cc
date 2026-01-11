#include "csp_demo.h"

int main() {
  // Create a buffered channel with capacity 2
  Channel<int> ch(2);

  // Producer thread
  std::thread producer([&ch]() {
    for (int i = 1; i <= 5; i++) {
      std::cout << "Sending: " << i << std::endl;
      ch.send(i);
      std::cout << "Sent: " << i << std::endl;
    }
    ch.close(); // No more data
  });

  // Consumer thread
  std::thread consumer([&ch]() {
    while (true) {
      auto value = ch.recv();
      if (!value) {
        std::cout << "Channel closed" << std::endl;
        break;
      }
      std::cout << "Received: " << *value << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  producer.join();
  consumer.join();

  return 0;
}