#pragma once

#include <cinttypes>

namespace ss
{
  enum class OpCode : std::uint8_t
  {
    NO_OP,
    CONSTANT,
    ADD,
    SUB,
    MUL,
    DIV,
    NEGATE,
    RETURN,
  };

  struct Instruction
  {
    OpCode      major_opcode   = OpCode::NO_OP;
    std::size_t modifying_bits = 0;
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
      case OpCode::ADD: {
        return "ADD";
      } break;
      case OpCode::SUB: {
        return "SUB";
      } break;
      case OpCode::MUL: {
        return "MUL";
      } break;
      case OpCode::DIV: {
        return "DIV";
      } break;
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