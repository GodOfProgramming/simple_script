#pragma once

#include "datatypes.hpp"
#include "code.hpp"
#include <vector>

namespace ss
{
  class Chunk
  {
   public:
    void write(Instruction, std::size_t line);
    void write_constant(Value v, std::size_t line);

    auto constant_at(std::size_t offset) -> Value;

    void push_stack(Value v);
    auto pop_stack() -> Value;
    auto stack_empty() -> bool;

    auto line_at(std::size_t offset) const noexcept -> std::size_t;

    std::vector<Instruction> code;
    std::vector<Value>       constants;
    std::vector<Value>       stack;

   private:
    std::vector<std::size_t> lines;
    std::size_t              last_line            = 0;
    std::size_t              instructions_on_line = 0;

    void add_line(std::size_t line) noexcept;
  };
}  // namespace ss