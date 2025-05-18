#include <execution/just.hpp>
#include <execution/schedulers/inline_scheduler.hpp>
#include <execution/schedulers/thread_pool_scheduler.hpp>
#include <execution/then.hpp>

#include <iostream>
#include <thread>

struct print_receiver {
  template <class... Ts> void set_value(Ts &&...xs) {
    std::cout << "thread " << std::this_thread::get_id() << ": ";
    ((std::cout << xs << ' '), ...);
    std::cout << '\n';
  }
  // void set_error(std::exception_ptr) noexcept { std::terminate(); }
  void set_stopped() noexcept {}
};

int main() {
  using namespace execution;

  // 1.  Inline scheduler – runs on the caller’s thread
  auto inl_sched = inline_scheduler{};
  auto inl_sender =
      inl_sched.schedule() | then([] { return "hello from inline"; });
  inl_sender.connect(print_receiver{}).start();

  // 2.  Thread‑pool scheduler – runs on a worker thread
  thread_pool pool(std::thread::hardware_concurrency());
  auto pool_sched = pool.get_scheduler();

  auto pool_sender = pool_sched.schedule() | then([] { return 21; }) |
                     then([](int v) { return v * 2; });

  pool_sender.connect(print_receiver{}).start();

  // give the pool a moment to finish before main exits
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
