#pragma once

#include "myexecution/concepts.hpp"
#include <algorithm>
#include <functional>
#include <utility>

namespace execution {
template <execution::receiver Receiver, typename Function> class then_receiver {
public:
  then_receiver(Receiver receiver, Function function)
      : out_receiver_{std::move(receiver)}, function_{std::move(function)} {}

  template <typename... Ts> auto set_value(Ts &&...args) -> void {
    out_receiver_.set_value(std::invoke(function_, std::forward<Ts>(args)...));
  }

  void set_error(std::exception_ptr ep) noexcept {
    out_receiver_.set_error(std::move(ep));
  }

  void set_stopped() noexcept { out_receiver_.set_stopped(); }

private:
  Receiver out_receiver_;
  Function function_;
};

template <execution::sender Sender, typename Function> class then_sender {
public:
  using sender_concept = execution::sender_t;
  then_sender(Sender inner_sender, Function function)
      : inner_sender_{std::move(inner_sender)}, function_{std::move(function)} {
  }

  template <execution::receiver Receiver> auto connect(Receiver receiver) {
    auto wrapped = then_receiver{std::move(receiver), function_};
    return inner_sender_.connect(std::move(wrapped));
  }

private:
  Sender inner_sender_;
  Function function_;
};

template <execution::sender Sender, typename Function>
auto then(Sender sender, Function function) -> then_sender<Sender, Function> {
  return then_sender{std::move(sender), std::move(function)};
}

template <typename Function> class then_closure {
public:
  then_closure(Function function) : function_{std::move(function)} {}

  template <execution::sender Sender>
  auto operator()(Sender &&sender) -> then_sender<Sender, Function> {
    return then(std::forward<Sender>(sender), function_);
  }

private:
  Function function_;
};

template <typename Function> auto then(Function function) {
  return then_closure{std::move(function)};
}

template <execution::sender Sender, typename Closure>
auto operator|(Sender &&sender, Closure &&closure)
    -> decltype(closure(std::forward<Sender>(sender))) {
  return closure(std::forward<Sender>(sender));
}
}; // namespace execution
