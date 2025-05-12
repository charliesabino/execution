#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
namespace toy {

class thread_pool {
private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  bool done_ = false;

  auto run() -> void {
    std::function<void()> task{};
    {
      std::unique_lock guard(mtx_);
      cv_.wait(guard,
               [this]() { return this->done_ || !this->tasks_.empty(); });
      if (done_ || tasks_.empty()) {
        return;
      }
      auto task = tasks_.front();
      tasks_.pop();
    }
    task();
  }

public:
  thread_pool(auto num_workers = std::thread::hardware_concurrency()) {
    while (num_workers--) {
      workers_.emplace_back([this]() { run(); });
    }
  }

  // Notably, this enables currently running tasks to continue running, even
  // after the thread pool exits scope. This may raise the concern of dangling
  // pointers—the worker threads contain a pointer to the thread pool, which may
  // expire before the thread exits. However, the code ensures that this pointer
  // is never accessed.
  ~thread_pool() {
    {
      std::lock_guard guard(mtx_);
      done_ = true;
    }
    cv_.notify_all();
    for (auto &worker : workers_) {
      worker.join();
    }
  }

  auto post(std::function<void()> task) -> void {
    std::lock_guard guard(mtx_);
    tasks_.emplace(std::move(task));
    cv_.notify_one();
  };

public:
  class scheduler {
  private:
    thread_pool *pool_;

  public:
    scheduler(thread_pool *pool) : pool_{pool} {}
  };
  auto get_scheduler() -> scheduler { return scheduler{this}; }
};
}; // namespace toy
