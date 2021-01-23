#pragma once

#include <string>

namespace ss
{
  namespace util
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

    auto load_file_to_string(std::string filename) -> std::string;
  }  // namespace util
}  // namespace ss