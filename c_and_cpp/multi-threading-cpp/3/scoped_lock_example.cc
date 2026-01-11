#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

class BankAccount {
public:
  std::string name;
  int balance;
  std::mutex mu;

  BankAccount(std::string n, int b) : name(n), balance(b) {}
};

// ❌ WRONG: Possible deadlock
void transfer_unsafe(BankAccount &from, BankAccount &to, int amount) {
  std::lock_guard<std::mutex> lock1(from.mu); // Lock first
  std::this_thread::sleep_for(
      std::chrono::microseconds(1));        // Increase deadlock chance
  std::lock_guard<std::mutex> lock2(to.mu); // Lock second - DEADLOCK RISK!

  if (from.balance >= amount) {
    from.balance -= amount;
    to.balance += amount;
    std::cout << "Transfer $" << amount << " from " << from.name << " to "
              << to.name << "\n";
  }
}

// ✅ CORRECT: Using scoped_lock
void transfer_safe(BankAccount &from, BankAccount &to, int amount) {
  std::scoped_lock lock(from.mu, to.mu); // Lock both atomically

  if (from.balance >= amount) {
    from.balance -= amount;
    to.balance += amount;
    std::cout << "Transfer $" << amount << " from " << from.name << " to "
              << to.name << "\n";
  }
}

// ✅ CORRECT: C++11 alternative (verbose)
void transfer_safe_cpp11(BankAccount &from, BankAccount &to, int amount) {
  std::lock(from.mu, to.mu); // Lock both, deadlock-free
  std::lock_guard<std::mutex> lock1(from.mu, std::adopt_lock);
  std::lock_guard<std::mutex> lock2(to.mu, std::adopt_lock);

  if (from.balance >= amount) {
    from.balance -= amount;
    to.balance += amount;
    std::cout << "Transfer $" << amount << " from " << from.name << " to "
              << to.name << "\n";
  }
}

void print_balances(BankAccount &a, BankAccount &b) {
  std::cout << "\n====== Balances ======\n";
  std::cout << a.name << ": $" << a.balance << "\n";
  std::cout << b.name << ": $" << b.balance << "\n";
  std::cout << "Total: $" << (a.balance + b.balance) << "\n";
  std::cout << "======================\n\n";
}

int main() {
  BankAccount alice("Alice", 1000);
  BankAccount bob("Bob", 1000);

  std::cout << "Initial state:";
  print_balances(alice, bob);

  // Many threads transferring in both directions
  std::vector<std::thread> threads;

  // Alice -> Bob transfers
  for (int i = 0; i < 5; i++) {
    threads.emplace_back([&]() {
      for (int j = 0; j < 10; j++) {
        transfer_safe(alice, bob, 10);
      }
    });
  }

  // Bob -> Alice transfers (opposite direction!)
  for (int i = 0; i < 5; i++) {
    threads.emplace_back([&]() {
      for (int j = 0; j < 10; j++) {
        transfer_safe(bob, alice, 10);
      }
    });
  }

  // Wait for all
  for (auto &t : threads) {
    t.join();
  }

  std::cout << "\nFinal state:";
  print_balances(alice, bob);

  std::cout << "✅ No deadlock! Total preserved!\n";

  return 0;
}