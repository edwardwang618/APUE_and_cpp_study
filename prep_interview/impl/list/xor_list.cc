#include <iostream>

struct Node {
  int x;
  void *xor_;
  Node(int x) : x(x), xor_(nullptr) {}
};

int main() {
  Node *head = new Node(0);
  head->xor_ = new Node(1);
  Node *n1 = static_cast<Node*>(head->xor_);
  n1->xor_ = head;

  std::cout << head->x << std::endl;
  std::cout << n1->x << std::endl;

  void* p = reinterpret_cast<long long>(head) ^ reinterpret_cast<long long>(n1);
}