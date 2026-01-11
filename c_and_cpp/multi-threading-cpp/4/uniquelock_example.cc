#include <iostream>
#include <mutex>
#include <thread>

std::mutex m;

void demo_basic() {
  std::cout << "=== Basic Usage ===\n";

  std::unique_lock<std::mutex> ul(m); // Locks immediately
  std::cout << "Mutex locked\n";
  std::cout << "Owns lock? " << ul.owns_lock() << "\n";

} // Automatically unlocks here

void demo_deferred_locking() {
  std::cout << "\n=== Deferred Locking ===\n";

  std::unique_lock<std::mutex> ul(m, std::defer_lock);  // Doesn't lock yet!
  std::cout << "Owns lock? " << ul.owns_lock() << "\n"; // 0 (false)

  // Do some non-critical work...
  std::cout << "Doing non-critical work...\n";

  ul.lock();                                            // Now we lock
  std::cout << "Owns lock? " << ul.owns_lock() << "\n"; // 1 (true)

  // Critical section here
}

void demo_early_unlock() {
  std::cout << "\n=== Early Unlock ===\n";

  std::unique_lock<std::mutex> ul(m);
  std::cout << "Mutex locked\n";

  // Critical section...

  ul.unlock(); // Unlock early!
  std::cout << "Mutex unlocked early\n";
  std::cout << "Owns lock? " << ul.owns_lock() << "\n"; // 0

  // Do more non-critical work without holding the lock
  std::cout << "Doing non-critical work...\n";

  ul.lock(); // Can relock if needed!
  std::cout << "Relocked!\n";
}

void demo_ownership_transfer() {
  std::cout << "\n=== Ownership Transfer (Why 'Unique') ===\n";

  std::unique_lock<std::mutex> ul1(m);
  std::cout << "ul1 owns lock? " << ul1.owns_lock() << "\n"; // 1

  // Transfer ownership to ul2
  std::unique_lock<std::mutex> ul2 = std::move(ul1);

  std::cout << "After move:\n";
  std::cout << "ul1 owns lock? " << ul1.owns_lock() << "\n"; // 0
  std::cout << "ul2 owns lock? " << ul2.owns_lock() << "\n"; // 1

  // ul1 is now empty - like a moved-from unique_ptr
  // ul2 will unlock when it goes out of scope
}

std::unique_lock<std::mutex> get_lock() {
  std::unique_lock<std::mutex> ul(m);
  // Do some setup...
  return ul; // Ownership transferred to caller!
}

void demo_return_from_function() {
  std::cout << "\n=== Return Lock from Function ===\n";

  std::unique_lock<std::mutex> ul = get_lock();
  std::cout << "Received lock, owns? " << ul.owns_lock() << "\n";

  // We now own the lock that was created in get_lock()
}

void demo_release() {
  std::cout << "\n=== Release (Give Up Ownership) ===\n";

  std::unique_lock<std::mutex> ul(m);
  std::cout << "Locked and owns\n";

  std::mutex *pm = ul.release(); // Give up ownership, DON'T unlock

  std::cout << "After release:\n";
  std::cout << "ul owns lock? " << ul.owns_lock() << "\n"; // 0
  std::cout << "But mutex is still locked!\n";

  // We must manually unlock now!
  pm->unlock();
  std::cout << "Manually unlocked\n";
}

void demo_try_lock() {
  std::cout << "\n=== Try Lock ===\n";

  std::unique_lock<std::mutex> ul(m, std::defer_lock);

  if (ul.try_lock()) {
    std::cout << "Got the lock!\n";
  } else {
    std::cout << "Couldn't get the lock\n";
  }
}

int main() {
  demo_basic();
  demo_deferred_locking();
  demo_early_unlock();
  demo_ownership_transfer();
  demo_return_from_function();
  demo_release();
  demo_try_lock();

  return 0;
}