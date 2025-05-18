#pragma once

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
      cv_.wait(guard, [this]() { return done_ || !tasks_.empty(); });
      if (done_ || tasks_.empty()) {
        return;
      }
      task = tasks_.front();
      tasks_.pop();
    }
    task();
  }

public:
  explicit thread_pool(auto num_workers = std::thread::hardware_concurrency()) {
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
    {
      std::lock_guard guard(mtx_);
      tasks_.emplace(std::move(task));
    }
    cv_.notify_one();
  };

public:
  class scheduler {
  private:
    thread_pool &pool_;

  public:
    explicit scheduler(thread_pool &pool) : pool_{pool} {}

    class schedule_sender {
      thread_pool &pool_;

    public:
      friend scheduler;
      explicit schedule_sender(thread_pool &p) : pool_{p} {}

      template <class Receiver> class op_state {
        thread_pool &pool_;
        Receiver recv_;

      public:
        op_state(thread_pool &pool, Receiver recv)
            : pool_{pool}, recv_{std::move(recv)} {}

        auto start() noexcept -> void {
          pool_.post([r = std::move(recv_)]() mutable { r.set_value(); });
        }
      };

      template <class Receiver> auto connect(Receiver r) const {
        return op_state<Receiver>{pool_, std::move(r)};
      };
    };

    auto schedule() const noexcept -> schedule_sender {
      return schedule_sender{pool_};
    }
  };

  // "deducing this" wasn't not convered in class, but I have seen
  // it in conference talks and wanted to practice using it. Doing so eliminates
  // the unecessary space overhead of more pointer copies (although the compiler
  // might optimize them away anyways). Generally, I felt that sticking to
  // references over pointers was a good idea.
  auto get_scheduler(this thread_pool &self) -> scheduler {
    return scheduler{self};
  }
};
}; // namespace toy
