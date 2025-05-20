#pragma once

#include "myexecution/concepts.hpp"
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>

namespace execution {

template <typename... Ts> class just_sender {
  using sender_concept = execution::sender_t;

  template <execution::receiver Receiver> class op_state {
    Receiver receiver_;
    std::tuple<Ts...> vals_;

  public:
    op_state(Receiver receiver, std::tuple<Ts...> vals)
        : receiver_{std::move(receiver)}, vals_{std::move(vals)} {}

    auto start() -> void {
      std::apply([this](Ts &...xs) { receiver_.set_value(std::move(xs)...); },
                 vals_);
    }
  };

public:
  explicit just_sender(Ts... xs) : vals_(std::move(xs)...) {}

  template <execution::receiver Receiver>
  auto connect(Receiver receiver) -> op_state<Receiver> {
    return op_state{std::move(receiver), std::move(vals_)};
  }

private:
  std::tuple<Ts...> vals_;
};

template <typename... Ts> auto just(Ts &&...args) {
  return just_sender<std::decay_t<Ts>...>{std::forward<Ts>(args)...};
}
}; // namespace execution
