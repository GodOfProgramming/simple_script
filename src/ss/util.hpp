#pragma once

#include <string>
#include <sstream>
#include <iostream>

namespace ss
{
  enum class OpCode : std::uint8_t
  {
    NO_OP,
    CONSTANT,
    NEGATE,
    RETURN,
  };

  constexpr auto to_string(OpCode op) noexcept -> const char*
  {
    switch (op) {
      case OpCode::NO_OP: {
        return "NOOP";
      }
      case OpCode::CONSTANT: {
        return "CONSTANT";
      }
      case OpCode::NEGATE: {
        return "NEGATE";
      }
      case OpCode::RETURN: {
        return "RETURN";
      }
      default: {
        return "UNKNOWN";
      }
    }
  }
}  // namespace ss