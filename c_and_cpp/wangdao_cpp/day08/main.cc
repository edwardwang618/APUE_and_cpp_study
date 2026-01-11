#include "Logger.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <string>

// Simulate some business logic
class UserService {
public:
  bool login(const std::string &username, const std::string &password) {
    LOG_INFO_F("User '%s' attempting to login", username.c_str());

    // Simulate validation
    if (username.empty() || password.empty()) {
      LOG_WARN_F("Login failed: empty credentials for user '%s'",
                 username.c_str());
      return false;
    }

    if (password.length() < 6) {
      LOG_WARN_F("Login failed: password too short for user '%s'",
                 username.c_str());
      return false;
    }

    // Simulate successful login
    LOG_INFO_F("User '%s' logged in successfully", username.c_str());
    return true;
  }

  void logout(const std::string &username) {
    LOG_INFO_F("User '%s' logged out", username.c_str());
  }
};

class OrderService {
public:
  int createOrder(int userId, double amount) {
    static int orderId = 1000;
    orderId++;

    LOG_INFO_F("Creating order #%d for user %d, amount: $%.2f", orderId, userId,
               amount);

    if (amount <= 0) {
      LOG_ERROR_F("Order #%d failed: invalid amount %.2f", orderId, amount);
      return -1;
    }

    if (amount > 10000) {
      LOG_WARN_F("Order #%d: large amount $%.2f, requires review", orderId,
                 amount);
    }

    LOG_INFO_F("Order #%d created successfully", orderId);
    return orderId;
  }
};

// Simulate system events
void systemMonitor() {
  LOG_DEBUG("System monitor started");

  // Simulate checking system resources
  int cpuUsage = 45;
  int memUsage = 62;

  LOG_DEBUG_F("CPU usage: %d%%, Memory usage: %d%%", cpuUsage, memUsage);

  if (cpuUsage > 80) {
    LOG_WARN_F("High CPU usage: %d%%", cpuUsage);
  }

  if (memUsage > 90) {
    LOG_ERROR_F("Critical memory usage: %d%%", memUsage);
  }
}

// Multi-threaded logging demo
void workerThread(int threadId) {
  for (int i = 0; i < 3; i++) {
    LOG_INFO_F("Thread %d: processing task %d", threadId, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  LOG_DEBUG_F("Thread %d: finished", threadId);
}

int main() {
  std::cout << "========================================\n";
  std::cout << "       Logger Demo\n";
  std::cout << "========================================\n\n";

  // Configure logger
  Logger::getInstance().setOutputFile("server.log");
  Logger::getInstance().setConsoleOutput(true);
  Logger::getInstance().setLevel(LogLevel::DEBUG);

  // System startup
  LOG_INFO("=== Server Starting ===");
  LOG_INFO("Initializing components...");

  // System logs
  LOG_DEBUG("Loading configuration...");
  LOG_INFO("Configuration loaded");
  LOG_DEBUG("Connecting to database...");
  LOG_INFO("Database connection established");

  std::cout << "\n--- System Monitor ---\n";
  systemMonitor();

  // Business logs - User Service
  std::cout << "\n--- User Service Demo ---\n";
  UserService userService;

  userService.login("", "password");        // Empty username
  userService.login("john", "123");         // Short password
  userService.login("john", "password123"); // Success
  userService.logout("john");

  // Business logs - Order Service
  std::cout << "\n--- Order Service Demo ---\n";
  OrderService orderService;

  orderService.createOrder(1, 150.50);   // Normal order
  orderService.createOrder(2, -50);      // Invalid amount
  orderService.createOrder(3, 15000.00); // Large order

  // Multi-threaded demo
  std::cout << "\n--- Multi-threaded Demo ---\n";
  LOG_INFO("Starting worker threads...");

  std::vector<std::thread> threads;
  for (int i = 0; i < 3; i++) {
    threads.emplace_back(workerThread, i);
  }

  for (auto &t : threads) {
    t.join();
  }

  LOG_INFO("All worker threads completed");

  // Error simulation
  std::cout << "\n--- Error Simulation ---\n";
  LOG_ERROR("Database connection lost!");
  LOG_WARN("Attempting to reconnect...");
  LOG_INFO("Database reconnected");

  // Fatal error simulation
  LOG_FATAL("Disk space critical: 99% used!");

  // Shutdown
  std::cout << "\n--- Shutdown ---\n";
  LOG_INFO("=== Server Shutting Down ===");

  std::cout << "\n========================================\n";
  std::cout << "Demo complete! Check 'server.log' file.\n";
  std::cout << "========================================\n";

  return 0;
}