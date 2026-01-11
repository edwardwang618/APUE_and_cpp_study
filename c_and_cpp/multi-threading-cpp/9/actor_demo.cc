#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class Actor {
public:
  // ============================================
  // Constructor: create the worker thread
  // The actor starts running immediately
  // ============================================
  Actor() : stop_(false) {
    // Launch worker thread, runs the run() method
    worker_ = std::thread([this]() { run(); });
  }

  // ============================================
  // Destructor: gracefully stop the actor
  // ============================================
  ~Actor() {
    // Send a "poison pill" message to stop the loop
    // This ensures all previous messages are processed first
    send([this]() { stop_ = true; });

    // Wait for worker thread to finish
    worker_.join();
  }

  // ============================================
  // Send a message to the actor's mailbox
  // Can be called from any thread (thread-safe)
  // ============================================
  void send(std::function<void()> msg) {
    std::lock_guard<std::mutex> lk(mu_); // Lock the mailbox
    mailbox_.push(std::move(msg));       // Add message to queue
    cv_.notify_one();                    // Wake up worker if sleeping
  }

private:
  // ============================================
  // Main loop: runs in worker thread
  // Processes messages one by one
  // ============================================
  void run() {
    while (true) {
      std::function<void()> msg;

      // Step 1: Get next message from mailbox
      {
        std::unique_lock<std::mutex> lk(mu_);

        // Sleep until mailbox has messages
        cv_.wait(lk, [this]() { return !mailbox_.empty(); });

        // Take message from front of queue
        msg = std::move(mailbox_.front());
        mailbox_.pop();

        // Lock released here
      }

      // Step 2: Execute message (outside lock!)
      msg();

      // Step 3: Check if we should stop
      if (stop_)
        break;
    }
  }

  bool stop_;                                 // Flag to stop the loop
  std::thread worker_;                        // The worker thread
  std::queue<std::function<void()>> mailbox_; // Message queue
  std::mutex mu_;                             // Protects mailbox_
  std::condition_variable cv_;                // For sleeping/waking
};

int main() {
  Actor actor;

  // Send messages to actor
  actor.send([]() { std::cout << "Message 1" << std::endl; });

  actor.send([]() { std::cout << "Message 2" << std::endl; });

  actor.send([]() { std::cout << "Message 3" << std::endl; });

  // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return 0;
}