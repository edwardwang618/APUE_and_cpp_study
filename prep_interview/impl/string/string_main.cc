#include "my_string.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  // ==================== Construction ====================

  // Default constructor
  String s1;
  assert(s1.size() == 0);
  assert(s1.empty());

  // From C-string
  String s2("hello");
  assert(s2.size() == 5);
  assert(!s2.empty());

  // From C-string with length
  String s3("hello world", 5);
  assert(s3.size() == 5);

  // Fill constructor
  String s4(5, 'x');
  assert(s4.size() == 5);

  // Copy constructor
  String s5(s2);
  assert(s5.size() == 5);

  // Move constructor
  String s6(std::move(String("temp")));
  assert(s6.size() == 4);

  // ==================== Assignment ====================

  String s7;
  s7 = s2; // Copy assignment
  assert(s7.size() == 5);

  String s8;
  s8 = std::move(s7); // Move assignment
  assert(s8.size() == 5);

  String s9;
  s9 = "world"; // From C-string
  assert(s9.size() == 5);

  String s10;
  s10 = 'A'; // From char
  assert(s10.size() == 1);

  // ==================== Element Access ====================

  String s11("abcde");
  assert(s11[0] == 'a');
  assert(s11[4] == 'e');
  s11[0] = 'A';
  assert(s11[0] == 'A');

  assert(s11.at(1) == 'b');
  s11.at(1) = 'B';
  assert(s11.at(1) == 'B');

  assert(s11.front() == 'A');
  assert(s11.back() == 'e');

  const String s12("test");
  assert(s12[0] == 't');    // const version
  assert(s12.at(1) == 'e'); // const version

  // at() should throw on out of bounds
  bool threw = false;
  try {
    s11.at(100);
  } catch (const std::out_of_range &) {
    threw = true;
  }
  assert(threw);

  // ==================== C-String Access ====================

  String s13("hello");
  assert(strcmp(s13.c_str(), "hello") == 0);
  assert(strcmp(s13.data(), "hello") == 0);

  // ==================== Capacity ====================

  String s14;
  assert(s14.empty());
  s14 = "abc";
  assert(!s14.empty());
  assert(s14.size() == 3);
  assert(s14.length() == 3);
  assert(s14.capacity() >= 3);

  s14.reserve(100);
  assert(s14.capacity() >= 100);

  s14.shrink_to_fit();
  assert(s14.capacity() >= s14.size());

  // ==================== Modifiers ====================

  String s15("hello");
  s15.clear();
  assert(s15.empty());

  String s16("hello");
  s16.push_back('!');
  assert(s16.size() == 6);
  assert(s16.back() == '!');

  s16.pop_back();
  assert(s16.size() == 5);

  String s17("hello");
  s17.append(" world");
  assert(s17.size() == 11);

  String s18("hello");
  s18.append(String(" there"));
  assert(s18.size() == 11);

  String s19("hello");
  s19 += " world";
  assert(s19.size() == 11);

  s19 += String("!");
  assert(s19.size() == 12);

  s19 += '?';
  assert(s19.size() == 13);

  String s20("hello");
  s20.resize(3);
  assert(s20.size() == 3);

  s20.resize(6, 'x');
  assert(s20.size() == 6);
  assert(s20[5] == 'x');

  // ==================== Concatenation ====================

  String a("hello");
  String b(" world");

  String c = a + b;
  assert(c.size() == 11);

  String d = a + " there";
  assert(d.size() == 11);

  String e = "say " + a;
  assert(e.size() == 9);

  // ==================== Comparison ====================

  String cmp1("abc");
  String cmp2("abc");
  String cmp3("abd");
  String cmp4("ab");

  assert(cmp1 == cmp2);
  assert(cmp1 != cmp3);
  assert(cmp1 < cmp3);
  assert(cmp3 > cmp1);
  assert(cmp4 < cmp1);
  assert(cmp1 >= cmp2);
  assert(cmp1 <= cmp2);

  assert(cmp1 == "abc");
  assert("abc" == cmp1);
  assert(cmp1 != "xyz");
  assert(cmp1 < "abd");

  // ==================== Substring & Search ====================

  String s21("hello world");

  String sub = s21.substr(0, 5);
  assert(sub == "hello");

  sub = s21.substr(6);
  assert(sub == "world");

  assert(s21.find('o') == 4);
  assert(s21.find('o', 5) == 7);
  assert(s21.find('z') == String::npos);

  assert(s21.find("wor") == 6);
  assert(s21.find("xyz") == String::npos);

  // ==================== Iterators ====================

  String s22("abc");

  auto it = s22.begin();
  assert(*it == 'a');
  ++it;
  assert(*it == 'b');

  assert(*(s22.end() - 1) == 'c');

  // Range-based for
  String result;
  for (char c : s22) {
    result.push_back(c);
  }
  assert(result == "abc");

  // Const iterators
  const String s23("xyz");
  auto cit = s23.begin();
  assert(*cit == 'x');

  // ==================== Stream I/O ====================

  String s24("test");
  std::cout << s24 << "\n"; // Should print: test

  // ==================== Swap ====================

  String sw1("aaa");
  String sw2("bbb");
  sw1.swap(sw2);
  assert(sw1 == "bbb");
  assert(sw2 == "aaa");

  std::cout << "All tests passed!\n";
  return 0;
}