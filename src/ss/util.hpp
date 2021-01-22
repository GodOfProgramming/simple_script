#pragma once

namespace ss
{
  template <typename T>
  auto and_with(const T&& v) -> bool
  {
    return v;
  }

  template <typename T, typename... Ts>
  auto and_width(const T&& v, const Ts&&... rest) -> bool
  {
    return v && and_with(rest...);
  }
}  // namespace ss