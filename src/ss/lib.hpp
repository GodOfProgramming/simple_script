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
    using GlobalMap = std::unordered_map<Value::StringType, Value>;

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
    BytecodeChunk*              chunk;
    BytecodeChunk::CodeIterator ip;
    GlobalMap                   globals;

    void run();

    void interpret(BytecodeChunk& chunk);
    void run_chunk();

    void disassemble_chunk(std::string name, BytecodeChunk& chunk) noexcept;
    void disassemble_instruction(BytecodeChunk& chunk, Instruction i, std::size_t offset) noexcept;
  };
}  // namespace ss