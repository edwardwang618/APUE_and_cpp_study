#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void myFunction(std::promise<int> prom) {
  std::cout << "Producer: working..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  prom.set_value(42);
  std::cout << "Producer: value set!" << std::endl;
}

void threadFunction(std::shared_future<int> fut, int id) {
  std::cout << "Thread " << id << ": waiting..." << std::endl;
  int value = fut.get(); // all threads can call get()
  std::cout << "Thread " << id << ": got value = " << value << std::endl;
}

void basic_demo() {
  std::cout << "=== Basic Demo ===" << std::endl;

  std::promise<int> promise;
  std::shared_future<int> future = promise.get_future();

  std::thread myThread1(myFunction, std::move(promise));
  std::thread myThread2(threadFunction, future, 2);
  std::thread myThread3(threadFunction, future, 3);

  myThread1.join();
  myThread2.join();
  myThread3.join();
}

void future_vs_shared_future() {
  std::cout << "\n=== future vs shared_future ===" << std::endl;

  // std::future - can only get() once
  {
    std::promise<int> p;
    std::future<int> f = p.get_future();
    p.set_value(10);

    std::cout << "future.get() first: " << f.get() << std::endl;
    // std::cout << f.get();  // CRASH! can only call once
  }

  // std::shared_future - can get() multiple times
  {
    std::promise<int> p;
    std::shared_future<int> f = p.get_future();
    p.set_value(10);

    std::cout << "shared_future.get() first: " << f.get() << std::endl;
    std::cout << "shared_future.get() second: " << f.get() << std::endl;
    std::cout << "shared_future.get() third: " << f.get() << std::endl;
  }
}

void broadcast_demo() {
  std::cout << "\n=== Broadcast Demo ===" << std::endl;

  std::promise<void> start_signal;
  std::shared_future<void> start = start_signal.get_future();

  auto worker = [](std::shared_future<void> signal, int id) {
    signal.get(); // wait for start
    std::cout << "Worker " << id << " started at "
              << std::chrono::system_clock::now().time_since_epoch().count()
              << std::endl;
  };

  std::thread t1(worker, start, 1);
  std::thread t2(worker, start, 2);
  std::thread t3(worker, start, 3);

  std::cout << "Main: preparing..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "Main: GO!" << std::endl;
  start_signal.set_value(); // all threads start simultaneously

  t1.join();
  t2.join();
  t3.join();
}

int main() {
  basic_demo();
  future_vs_shared_future();
  broadcast_demo();
  return 0;
}