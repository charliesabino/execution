#include "toy.hpp"
#include <algorithm>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

struct PrintReceiver {

  template<typename ...Ts>
  auto set_value(Ts... args) -> void {
    ((std::cout << args << std::endl), ...);
  }
};

int main() {

  auto sender = toy::just(5, 7, "test");
  auto op_state = sender.connect(PrintReceiver{});
  op_state.start();
}
