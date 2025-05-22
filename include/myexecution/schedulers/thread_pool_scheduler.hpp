#pragma once
#include "myexecution/concepts.hpp"
#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace execution {
class thread_pool {
private:
  std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  bool done_ = false;

  auto run() -> void {
    for (;;) {
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
  }

public:
  explicit thread_pool(auto num_workers = std::thread::hardware_concurrency()) {
    while (num_workers--) {
      workers_.emplace_back([this]() { run(); });
    }
  }

  // Notably, this enables currently running tasks to continue running, even
  // after the thread pool exits scope. This may raise the concern of dangling
  // references—the worker threads contain a reference to the thread pool, which
  // may expire before the thread exits. However, the code ensures that this
  // reference is never accessed. Thus, we maintain our invariant.
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
      thread_pool *pool_;

    public:
      using sender_concept = execution::sender_t;

      schedule_sender(schedule_sender const &) noexcept = default;
      schedule_sender &operator=(schedule_sender const &) noexcept = default;
      schedule_sender(schedule_sender &&) noexcept = default;
      schedule_sender &operator=(schedule_sender &&) noexcept = default;

      explicit schedule_sender(thread_pool &p) noexcept
          : pool_{std::addressof(p)} {}

      template <class Receiver> class op_state {
        thread_pool &pool_;
        Receiver recv_;

      public:
        op_state(thread_pool &pool, Receiver recv)
            : pool_{pool}, recv_{std::move(recv)} {}

        void start() noexcept {
          pool_.post([r = std::move(recv_)]() mutable { r.set_value(); });
        }
      };

      template <class Receiver> auto connect(Receiver r) const {
        return op_state<Receiver>{*pool_, std::move(r)};
      }
    };

    auto schedule() const noexcept -> schedule_sender {
      return schedule_sender{pool_};
    }
  };

  // "deducing this" wasn't not convered in class, but I follow Barry/have seen
  // it in conference talks and wanted to practice using it. Doing so eliminates
  // the unecessary space overhead of more pointer copies (although maybe the
  // compiler will optimize them away anyways?). Generally, I felt that sticking
  // to references over pointers was a good idea.
  auto get_scheduler(this thread_pool &self) -> scheduler {
    return scheduler{self};
  }
};
}; // namespace execution
