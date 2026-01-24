#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>

template <typename T, size_t capacity> class BoundedQueue {
  std::mutex mu_;
  std::queue<T> q;
  std::condition_variable cv_push, cv_pop;

public:
  void push(const T &v) {
    std::unique_lock<std::mutex> lk(mu_);
    cv_push.wait(lk, [this] { return q.size() < capacity; });
    q.push(v);
    cv_pop.notify_one();
  }

  void push(T &&v) {
    std::unique_lock<std::mutex> lk(mu_);
    cv_push.wait(lk, [this] { return q.size() < capacity; });
    q.push(std::move(v));
    cv_pop.notify_one();
  }

  void pop(T &v) {
    std::unique_lock<std::mutex> lk(mu_);
    cv_pop.wait(lk, [this] { return !q.empty(); });
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "v is not move assignable!");
    v = std::move(q.front());
    q.pop();
    cv_push.notify_one();
  }

  bool try_pop(T &v) {
    std::unique_lock<std::mutex> lk(mu_);
    if (q.empty())
      return false;
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "v is not move assignable!");
    v = std::move(q.front());
    q.pop();
    cv_push.notify_one();
    return true;
  }
};

int main() {
  BoundedQueue<int, 5> bq;

  std::thread producer([&] {
    for (int i = 0; i < 20; i++) {
      bq.push(i);
      std::cout << "pushed " << i << "\n";
    }
  });

  std::thread consumer([&] {
    for (int i = 0; i < 20; i++) {
      int v;
      bq.pop(v);
      std::cout << "popped " << v << "\n";
    }
  });

  producer.join();
  consumer.join();
}