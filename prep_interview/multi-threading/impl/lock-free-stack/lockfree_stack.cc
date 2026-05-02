#include <algorithm>
#include <atomic>
#include <cstddef>

template <typename T> class Stack {
  struct Node {
    T val;
    Node *next;
  };
  struct TaggedPtr {
    Node *ptr;
    size_t tag;
  };
  std::atomic<TaggedPtr> head;

public:
  void push(const T &val) {
    auto h = head.load(std::memory_order_relaxed);
    TaggedPtr node{new Node{val, h.ptr}, h.tag + 1};
    while (!head.compare_exchange_weak(h, node, std::memory_order_release,
                                       std::memory_order_relaxed)) {
      node.ptr->next = h.ptr;
      node.tag = h.tag + 1;
    }
  }

  bool pop(T &val) {
    TaggedPtr h = head.load(std::memory_order_acquire);
    do {
      if (!h.ptr)
        return false;
    } while (!head.compare_exchange_weak(h, TaggedPtr{h.ptr->next, h.tag + 1},
                                         std::memory_order_release,
                                         std::memory_order_relaxed));
    val = std::move(h.ptr->val);
    // delete h.ptr;
    return true;
  }
};