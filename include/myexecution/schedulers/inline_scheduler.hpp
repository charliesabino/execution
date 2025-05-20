#pragma once

#include "myexecution/concepts.hpp"
#include <utility>

class inline_scheduler {
private:
  class schedule_sender {
    using sender_concept = execution::sender_t;

  private:
    template <execution::receiver Receiver> class op_state {
    private:
      Receiver receiver_;

    public:
      explicit op_state(Receiver receiver) : receiver_{std::move(receiver)} {}
      auto start() noexcept -> void { receiver_.set_value(); }
    };

  public:
    template <execution::receiver Receiver>
    auto connect(Receiver receiver) const -> op_state<std::decay_t<Receiver>> {
      return op_state{std::move(receiver)};
    };
  };

public:
  auto schedule() const -> schedule_sender { return schedule_sender{}; };
};
