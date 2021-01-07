#include "datatypes.hpp"
#include "ss/chunk.hpp"
#include "ss/util.hpp"
#include <cinttypes>

namespace ss
{
  class VM
  {
   public:
    VM();
    ~VM();

    auto run() -> InterpretResult;

   private:
    Chunk*                              chunk;
    std::vector<std::uint8_t>::iterator ip;

    auto interpret(Chunk* chunk) -> InterpretResult;

    void disassemble_instruction(std::size_t offset);
    void print_value(const Value& v);
  };
}  // namespace ss