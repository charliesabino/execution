#pragma once

#include <concepts>
#include <exception>
#include <utility>

namespace execution {

template <typename R>
concept receiver = requires(R &&r) {
  { std::forward<R>(r).set_value() };
  { std::forward<R>(r).set_error(std::exception_ptr{}) } noexcept;
  { std::forward<R>(r).set_stopped() } noexcept;
};
} // namespace execution
