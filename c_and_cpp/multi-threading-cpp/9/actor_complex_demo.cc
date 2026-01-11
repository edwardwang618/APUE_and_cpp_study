#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

// ============================================
// Base Actor class with message passing
// ============================================
class Actor {
public:
  Actor(std::string name) : name_(std::move(name)), stop_(false) {
    worker_ = std::thread([this]() { run(); });
  }

  virtual ~Actor() {
    send([this]() { stop_ = true; });
    worker_.join();
  }

  void send(std::function<void()> msg) {
    std::lock_guard<std::mutex> lk(mu_);
    mailbox_.push(std::move(msg));
    cv_.notify_one();
  }

  const std::string &name() const { return name_; }

private:
  void run() {
    while (true) {
      std::function<void()> msg;
      {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [this]() { return !mailbox_.empty(); });
        msg = std::move(mailbox_.front());
        mailbox_.pop();
      }
      msg();
      if (stop_)
        break;
    }
  }

  std::string name_;
  bool stop_;
  std::thread worker_;
  std::queue<std::function<void()>> mailbox_;
  std::mutex mu_;
  std::condition_variable cv_;
};

// ============================================
// Worker Pool: Master distributes work to workers
// ============================================

class Worker;

class Master : public Actor {
public:
  Master() : Actor("Master"), next_worker_(0) {}

  void add_worker(Worker *w) { workers_.push_back(w); }

  // Distribute work to workers (round-robin)
  void submit_job(int job_id);

  // Receive result from worker
  void receive_result(const std::string &worker_name, int job_id, int result) {
    send([=]() {
      std::cout << "[Master] Got result from " << worker_name << ": job "
                << job_id << " = " << result << std::endl;
    });
  }

private:
  std::vector<Worker *> workers_;
  size_t next_worker_;
};

class Worker : public Actor {
public:
  Worker(std::string name, Master *master)
      : Actor(std::move(name)), master_(master) {}

  void do_work(int job_id) {
    send([this, job_id]() {
      std::cout << "[" << name() << "] Processing job " << job_id << std::endl;

      // Simulate work
      int result = job_id * job_id;

      // Send result back to master
      master_->receive_result(name(), job_id, result);
    });
  }

private:
  Master *master_;
};

void Master::submit_job(int job_id) {
  send([this, job_id]() {
    // Round-robin distribution
    Worker *worker = workers_[next_worker_];
    next_worker_ = (next_worker_ + 1) % workers_.size();

    std::cout << "[Master] Assigning job " << job_id << " to " << worker->name()
              << std::endl;
    worker->do_work(job_id);
  });
}

int main() {
  std::cout << "\n===== Worker Pool Example =====" << std::endl;
  {
    Master master;
    Worker w0("Worker-0", &master);
    Worker w1("Worker-1", &master);
    Worker w2("Worker-2", &master);

    master.add_worker(&w0);
    master.add_worker(&w1);
    master.add_worker(&w2);

    // Submit jobs
    for (int i = 1; i <= 6; i++) {
      master.submit_job(i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}