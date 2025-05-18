#include "toy.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <thread>

struct PrintReceiver {

  template <typename... Ts> auto set_value(Ts... args) -> void {
    std::cout << "Thread ID: " << std::this_thread::get_id() << '\n';
    ((std::cout << args << std::endl), ...);
  }
};

int main() {

  auto sender = toy::just(5, 7, "test");
  sender.connect(PrintReceiver{}).start();

  auto sender2 = toy::just(6, 8, 10);
  toy::then(sender2, [](int x, int y, int z) { return x + y + z; })
      .connect(PrintReceiver{})
      .start();

  toy::then(toy::then(sender2, [](int x, int y, int z) { return x + y + z; }),
            [](int x) { return x / 2; })
      .connect(PrintReceiver{})
      .start();

  auto sender3 = toy::just(70) | toy::then([](auto x) { return x * 2; });
  sender3.connect(PrintReceiver{}).start();

  auto sched1 = toy::inline_scheduler{};
  auto pipe = sched1.schedule() | toy::then([]() { return 1; }) |
              toy::then([](auto x) { return x * 4; }) |
              toy::then([](auto y) { return y - 2; });
  pipe.connect(PrintReceiver{}).start();

  toy::thread_pool pool(std::thread::hardware_concurrency());
  auto sched2 = pool.get_scheduler();
  auto pipeline = sched2.schedule() | toy::then([] { return 21; }) |
                  toy::then([](int v) { return v * 2; });

  auto op = pipeline.connect(PrintReceiver{});
  op.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
