#pragma once

#include <concepts>
#include <condition_variable>
#include <cstddef>
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

  auto run() -> void;

public:
  explicit thread_pool(
      std::integral auto num_workers = std::thread::hardware_concurrency()) {
    while (num_workers--) {
      workers_.emplace_back([this]() { run(); });
    }
  }
  ~thread_pool();

  auto post(std::function<void()> task) -> void;

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

  auto get_scheduler(this thread_pool &self) -> scheduler;
};
}; // namespace execution
