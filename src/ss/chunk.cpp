#include "chunk.hpp"

namespace ss
{
  void Chunk::write(Instruction i)
  {
    this->code.push_back(i);
  }

  void Chunk::write_constant(Value v)
  {
    this->constants.push_back(v);
    Instruction i{
     OpCode::CONSTANT,
     this->constants.size() - 1,
    };
    this->write(i);
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
}  // namespace ss
