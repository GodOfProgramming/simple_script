#include "chunk.hpp"

namespace ss
{
  void Chunk::write(Instruction i, std::size_t line)
  {
    this->code.push_back(i);
    this->add_line(line);
  }

  void Chunk::write_constant(Value v, std::size_t line)
  {
    this->constants.push_back(v);
    Instruction i{
     OpCode::CONSTANT,
     this->constants.size() - 1,
    };
    this->write(i, line);
  }

  auto Chunk::constant_at(std::size_t offset) -> Value
  {
    return this->constants[offset];
  }

  void Chunk::push_stack(Value v)
  {
    this->stack.push_back(v);
  }

  auto Chunk::pop_stack() -> Value
  {
    Value v = this->stack.back();
    this->stack.pop_back();
    return v;
  }

  auto Chunk::stack_empty() -> bool
  {
    return this->stack.empty();
  }

  // increments the current number of instructions on a line
  // or publishes the number and resets the count
  void Chunk::add_line(std::size_t line) noexcept
  {
    if (this->last_line == line) {
      // same line number
      this->instructions_on_line += 1;
    } else {
      this->lines.push_back(this->instructions_on_line);
      this->last_line            = line;
      this->instructions_on_line = 1;  // current instruction
    }
  }

  // extracts the line at the given instruction offset
  auto Chunk::line_at(std::size_t offset) const noexcept -> std::size_t
  {
    std::size_t accum = 0;
    std::size_t line  = 0;
    for (const auto num_instructions_on_line : this->lines) {
      if (accum + num_instructions_on_line > offset) {
        return line;
      } else {
        accum += num_instructions_on_line;
      }
      line++;
    }
    return line;
  }
}  // namespace ss
