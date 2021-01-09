#pragma once

#include "datatypes.hpp"
#include "chunk.hpp"
#include "code.hpp"
#include <cinttypes>
#include <iostream>
#include <memory>

namespace ss
{
  class VMConfig
  {
   public:
    static VMConfig basic;

    VMConfig(std::istream* istream = &std::cin, std::ostream* ostream = &std::cout);
    ~VMConfig() = default;

    template <typename... Args>
    void write(Args&&... args)
    {
      if (this->ostream != nullptr) {
        (((*this->ostream) << std::forward<Args>(args)), ...);
      }
    }

    template <typename... Args>
    void write_line(Args&&... args)
    {
      write(args..., '\n');
    }

    template <typename T>
    void read(T& t)
    {
      if (this->istream != nullptr) {
        (*istream) >> t;
      }
    }

    void reset_istream();
    void reset_ostream();

   private:
    std::istream* istream;
    std::ostream* ostream;

    std::shared_ptr<std::ios> istream_initial_state;
    std::shared_ptr<std::ios> ostream_initial_state;
  };

  auto repl(VMConfig cfg = VMConfig::basic) -> int;
  class VM
  {
   public:
    VM(VMConfig cfg = VMConfig::basic);
    ~VM() = default;

    void test();

   private:
    using InstructionPointer = std::vector<Instruction>::iterator;

    VMConfig           config;
    Chunk*             chunk;
    InstructionPointer ip;

    void interpret(Chunk& chunk);
    void run();

    void disassemble_chunk(std::string name, Chunk& chunk) noexcept;
    void disassemble_instruction(Chunk& chunk, Instruction i, std::size_t offset) noexcept;

    void print_stack() noexcept;
  };
}  // namespace ss