#include "myexecution/concepts.hpp"
#include <myexecution/myexecution.hpp>

#include <iostream>
#include <thread>

struct print_receiver {
  template <class... Ts> void set_value(Ts &&...xs) {
    std::cout << "thread " << std::this_thread::get_id() << ": ";
    ((std::cout << xs << ' '), ...);
    std::cout << '\n';
  }
  void set_error(std::exception_ptr) noexcept { std::terminate(); }
  void set_stopped() noexcept {}
};

int main() {
  using namespace execution;

  // 1.  Inline scheduler – runs on the caller’s thread
  auto inl_sched = inline_scheduler{};
  sender auto inl_sender =
      inl_sched.schedule() | then([] { return "hello from inline"; });
  operation_state auto inl_op = inl_sender.connect(print_receiver{});
  inl_op.start();

  // note: you could do also just do
  // inl_sender.connect(print_receiver{}).start(); I stored the operation state
  // for illustrative purposes only. The same applies to the example below

  // 2.  Thread‑pool scheduler – runs on a worker thread
  thread_pool pool(std::thread::hardware_concurrency());
  auto pool_sched = pool.get_scheduler();

  sender auto pool_sender = pool_sched.schedule() | then([] { return 21; }) |
                            then([](int v) { return v * 2; });

  operation_state auto pool_op = pool_sender.connect(print_receiver{});
  pool_op.start();

  // give the pool a moment to finish before main exits
  // sync wait gave me too much trouble to implement :(
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
