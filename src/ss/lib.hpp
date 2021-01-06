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
    Chunk*        chunk;
    std::vector<std::uint8_t>::iterator ip;

    auto interpret(Chunk* chunk) -> InterpretResult;
  };
}  // namespace ss