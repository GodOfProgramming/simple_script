#include "lib.hpp"
#include <exception>
#include <iostream>
#include <sstream>

namespace
{
  constexpr bool SHOW_DISASSEMBLY = true;
}

namespace ss
{
  auto VM::run() -> InterpretResult
  {
    using Type = InterpretResult::Type;

    while (this->ip < this->chunk->code.end()) {
      if constexpr (SHOW_DISASSEMBLY) {
        std::cout << "          ";
        for (const auto& value : this->chunk->stack) {
          std::cout << "[ ";
          this->print_value(value);
          std::cout << " ]";
        }
        std::cout << '\n';
        this->disassemble_instruction(this->ip - this->chunk->code.begin());
      }
      switch (static_cast<OpCode>(*this->ip)) {
        case OpCode::CONSTANT: {
          if (++this->ip < this->chunk->code.end()) {
            Value constant = this->chunk->constants[*this->ip];
            this->chunk->stack.push_back(constant);
          } else {
            return InterpretResult(Type::RUNTIME_ERROR, "no value following constant opcode");
          }
        } break;
        case OpCode::RETURN: {
          if (!this->chunk->stack_empty()) {
            this->print_value(this->chunk->pop_stack());
          }
          return InterpretResult::ok();
        } break;
        default: {
          std::stringstream ss;
          ss << "invalid op code: " << static_cast<std::uint32_t>(*this->ip);
          return InterpretResult(Type::RUNTIME_ERROR, ss.str());
        }
      }
      this->ip++;
    }

    return InterpretResult::ok();
  }

  auto VM::interpret(Chunk* chunk) -> InterpretResult
  {
    this->chunk = chunk;
    this->ip    = chunk->code.begin();

    this->run();
  }

  void VM::print_value(const Value& value) {}
}  // namespace ss