#include <algorithm>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace toy {
template <typename... Ts> class just_sender {
public:
  template <typename Receiver> class op_state {
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

  template <typename Receiver>
  auto connect(Receiver receiver) -> op_state<Receiver> {
    return op_state{std::move(receiver), std::move(vals_)};
  }

private:
  std::tuple<Ts...> vals_;
};

template <typename... Ts> auto just(Ts &&...args) {
  return just_sender<std::decay_t<Ts>...>{std::forward<Ts>(args)...};
}

template <typename Receiver, typename Function> class then_receiver {
public:
  then_receiver(Receiver receiver, Function function)
      : out_receiver_{std::move(receiver)}, function_{std::move(function)} {}

  template <typename... Ts> auto set_value(Ts &&...args) -> void {
    out_receiver_.set_value(std::invoke(function_, std::forward<Ts>(args)...));
  }

private:
  Receiver out_receiver_;
  Function function_;
};

template <typename Sender, typename Function> class then_sender {
public:
  then_sender(Sender inner_sender, Function function)
      : inner_sender_{std::move(inner_sender)}, function_{std::move(function)} {
  }

  template <typename Receiver> auto connect(Receiver receiver) {
    auto wrapped = then_receiver{std::move(receiver), function_};
    return inner_sender_.connect(std::move(wrapped));
  }

private:
  Sender inner_sender_;
  Function function_;
};

template <typename Sender, typename Function>
auto then(Sender sender, Function function) -> then_sender<Sender, Function> {
  return then_sender{std::move(sender), std::move(function)};
}

template <typename Function> class then_closure {
public:
  then_closure(Function function) : function_{std::move(function)} {}

  template <typename Sender>
  auto operator()(Sender &&sender) -> then_sender<Sender, Function> {
    return then(std::forward<Sender>(sender), function_);
  }

private:
  Function function_;
};

template <typename Function> auto then(Function function) {
  return then_closure{std::move(function)};
}

template <typename Sender, typename Closure>
auto operator|(Sender &&sender, Closure &&closure) // we rely on ADL here!
    -> decltype(closure(std::forward<Sender>(sender))) {
  return closure(std::forward<Sender>(sender));
}

} // namespace toy
