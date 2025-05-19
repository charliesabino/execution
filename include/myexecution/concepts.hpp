#pragma once

#include <exception>
#include <utility>

namespace execution {

template <typename Recv>
concept receiver = requires(Recv &&r) {
  { std::forward<Recv>(r).set_value() };
  { std::forward<Recv>(r).set_error(std::exception_ptr{}) } noexcept;
  { std::forward<Recv>(r).set_stopped() } noexcept;
};

template <typename OpState>
concept operation_state = requires(OpState op) {
  { op.start() } noexcept;
};

template <typename Sender, typename Recv>
concept sender = receiver<Recv> && requires(Sender s, Recv r) {
  { connect(std::move(s), std::move(r)) } -> operation_state;
};

} // namespace execution
