#include "toy.hpp"
#include <iostream>

struct PrintReceiver {

  template <typename... Ts> auto set_value(Ts... args) -> void {
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
}
