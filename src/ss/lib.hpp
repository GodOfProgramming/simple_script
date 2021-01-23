#pragma once

#include "cfg.hpp"
#include "code.hpp"
#include "datatypes.hpp"
#include <cinttypes>
#include <unordered_map>

namespace ss
{
  class VM
  {
   public:
    VM(VMConfig cfg = VMConfig::basic);
    ~VM() = default;

    auto repl(VMConfig cfg = VMConfig::basic) -> int;

    void run_script(std::string src);

    void set_var(Value::StringType name, Value value) noexcept;
    auto get_var(Value::StringType name) noexcept -> Value;

    void test();

   private:
    VMConfig                    config;
    BytecodeChunk               chunk;
    BytecodeChunk::CodeIterator ip;

    void run_line(std::string line);
    void compile(std::string&& src);
    void execute();

    void disassemble_chunk() noexcept;
    void disassemble_instruction(Instruction i, std::size_t offset) noexcept;
  };
}  // namespace ss