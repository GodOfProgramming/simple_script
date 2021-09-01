#pragma once

#include <istream>

namespace ss
{
  namespace util
  {
    auto stream_to_string(std::istream& filename) -> std::string;
  }  // namespace util
}  // namespace ss