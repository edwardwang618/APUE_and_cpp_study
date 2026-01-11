#include <iostream>

#pragma pack(push, 1)
struct Node {
  int x;
  Node *next;
};
#pragma pack(pop)

int main() {
  std::cout << sizeof(Node) << std::endl;
  return 0;
}