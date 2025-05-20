#pragma once

#include <concepts>
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

// this feels a bit hacky, but its what the proposal seems to do :/
struct sender_t {};

template <typename S>
concept sender = std::movable<std::remove_cvref_t<S>> &&
                 requires(const std::remove_cvref_t<S> &s) {
                   typename std::remove_cvref_t<S>::sender_concept;
                 };

} // namespace execution
