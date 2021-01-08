#pragma once

#include "datatypes.hpp"
#include "util.hpp"
#include <vector>

namespace ss
{
  struct Instruction
  {
    OpCode      major_opcode   = OpCode::NO_OP;
    std::size_t modifying_bits = 0;
  };
  struct Chunk
  {
    std::vector<Instruction> code;
    std::vector<Value>       constants;
    std::vector<Value>       stack;

    void write(Instruction);
    void write_constant(Value v);

    auto constant_at(std::size_t offset) -> Value;

    void push_stack(Value v);
    auto pop_stack() -> Value;
    auto stack_empty() -> bool;
  };
}  // namespace ss