#include <cstdio>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <unordered_map>

std::set<std::string, std::less<>> s{"alice", "bob", "carol"};

int main() {
  if (auto it = s.find("bob"); it != s.end())
    std::cout << *it << std::endl;
  if (s.contains("alice"))
    puts("Found!");

  std::map<std::string, int, std::less<>> m;
  m["alice"] = 1;
  m["bob"] = 2;

  std::string_view key = "alice";
  if (m.contains(key))
    puts("Found!");

  struct StringHash {
    using is_transparent = void;
    size_t operator()(const char *s) const {
      return std::hash<std::string_view>{}(s);
    }
    size_t operator()(std::string_view s) const {
      return std::hash<std::string_view>{}(s);
    }
    size_t operator()(const std::string &s) const {
      return std::hash<std::string_view>{}(s);
    }
  };

  struct StringEq {
    using is_transparent = void;
    bool operator()(std::string_view a, std::string_view b) const {
      return a == b;
    }
  };

  std::unordered_map<std::string, int, StringHash, StringEq> mp;
  mp["hello"] = 1;

  mp.find("hello");                   // no temp std::string
  mp.find(std::string_view{"hello"}); // no temp std::string
}