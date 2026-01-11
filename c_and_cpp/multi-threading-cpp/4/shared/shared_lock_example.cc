#include <chrono>
#include <iostream>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class DNService {
public:
  DNService() {}

  // 读操作采用共享锁 (Read operations use shared lock)
  std::string QueryDNS(std::string dnsname) {
    std::shared_lock<std::shared_mutex> shared_locks(_shared_mtx);
    auto iter = _dns_info.find(dnsname);
    if (iter != _dns_info.end()) {
      return iter->second;
    }
    return "";
  }

  // 写操作采用独占锁 (Write operations use exclusive lock)
  void AddDNSInfo(std::string dnsname, std::string dnsentry) {
    std::lock_guard<std::shared_mutex> guard_locks(_shared_mtx);
    _dns_info.insert(std::make_pair(dnsname, dnsentry));
  }

private:
  std::map<std::string, std::string> _dns_info;
  mutable std::shared_mutex _shared_mtx;
};

int main() {
  DNService dns_service;

  // Writer thread - adds DNS entries
  std::thread writer([&dns_service]() {
    std::vector<std::pair<std::string, std::string>> entries = {
        {"google.com", "142.250.80.46"},
        {"github.com", "140.82.121.4"},
        {"amazon.com", "205.251.242.103"},
        {"microsoft.com", "20.70.246.20"},
        {"apple.com", "17.253.144.10"}};

    for (const auto &entry : entries) {
      std::cout << "[Writer] Adding: " << entry.first << " -> " << entry.second
                << "\n";
      dns_service.AddDNSInfo(entry.first, entry.second);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Multiple reader threads - query DNS concurrently
  std::vector<std::thread> readers;
  for (int i = 0; i < 3; ++i) {
    readers.emplace_back([&dns_service, i]() {
      std::vector<std::string> queries = {"google.com", "github.com",
                                          "amazon.com", "microsoft.com",
                                          "apple.com",  "unknown.com"};

      for (int j = 0; j < 3; ++j) {
        for (const auto &query : queries) {
          std::string result = dns_service.QueryDNS(query);
          if (!result.empty()) {
            std::cout << "[Reader " << i << "] " << query << " = " << result
                      << "\n";
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
      }
    });
  }

  // Wait for all threads
  writer.join();
  for (auto &reader : readers) {
    reader.join();
  }

  std::cout << "\n=== Final DNS Cache ===\n";
  for (const auto &name : {"google.com", "github.com", "amazon.com",
                           "microsoft.com", "apple.com"}) {
    std::cout << name << " -> " << dns_service.QueryDNS(name) << "\n";
  }

  return 0;
}