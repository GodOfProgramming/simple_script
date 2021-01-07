#include "datatypes.hpp"
#include <vector>

namespace ss
{
  struct Chunk
  {
    std::vector<std::uint8_t> code;
    std::vector<Value>        constants;
    std::vector<Value>        stack;

    void write(OpCode oc);
    void write_constant(Value v);

    void push_stack(Value v);
    auto pop_stack() -> Value;
    auto stack_empty() -> bool;
  };
}  // namespace ss