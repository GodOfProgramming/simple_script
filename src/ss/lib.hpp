#pragma once

#include "datatypes.hpp"
#include "chunk.hpp"
#include "util.hpp"
#include <cinttypes>

namespace ss
{
  class VM
  {
   public:
    VM() = default;
    ~VM() = default;

    void test();

   private:
    Chunk*                 chunk;
    std::vector<Instruction>::iterator ip;

    void interpret(Chunk& chunk);
    void run();

    void disassemble_chunk(std::string name, Chunk& chunk) const noexcept;
    void disassemble_instruction(Chunk& chunk, Instruction i, std::size_t offset) const noexcept;

    void print_stack() const noexcept;
  };
}  // namespace ss